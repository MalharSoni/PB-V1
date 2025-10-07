# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a **VEX V5 robotics competition codebase** (Team 839Y) built with:
- **PROS 3.8.3** (VEX V5 RTOS kernel)
- **LemLib 0.5.0** (motion control and odometry library)
- **OkapiLib 4.8.0** (utility library)

**Hardware:** VEX V5 Brain running ARM Cortex-A9 processor. The robot competes in VEX Robotics Competition games.

## Build & Development Commands

### Building
```bash
# Standard build (default target)
make

# Quick build (same as default)
make quick

# Clean build artifacts
make clean

# Upload to V5 brain (requires PROS CLI and connected robot)
pros upload

# Build and upload
pros build-upload
```

### Development Tools
```bash
# Open PROS terminal UI (for selecting serial ports, etc.)
prosv5 terminal

# View compilation database
cat compile_commands.json
```

**Note:** This project uses hot/cold linking (`USE_PACKAGE:=1` in Makefile) to optimize build times by separating library code from user code.

## Code Architecture

### Coordinate System & Motion (LemLib)
- **Coordinate system:** Origin at field center `(0, 0)`
  - X-axis: positive = forward/backward in robot's reference
  - Y-axis: positive = left/right in robot's reference
  - Units: **inches**
- **Heading:** Degrees, where `0°` = robot facing along positive X-axis
- **Key LemLib chassis methods:**
  - `chassis.setPose(x, y, theta)` - set odometry position
  - `chassis.getPose()` - returns `lemlib::Pose{x, y, theta}`
  - `chassis.moveToPoint(x, y, timeout, {options})` - move to field position
  - `chassis.turnToPoint(x, y, timeout, {options})` - turn to face point
  - `chassis.turnToHeading(theta, timeout)` - turn to absolute heading
  - `chassis.waitUntil(distance)` - wait until within distance of target
  - `chassis.waitUntilDone()` - blocking wait for motion completion

**Motion options:** `.forwards`, `.maxSpeed`, use in braces like `{.forwards = false, .maxSpeed = 80}`

### Directory Structure

```
include/
  robot/           # Subsystem header files
    auton.hpp      # Autonomous routines
    movement.hpp   # Driver control (arcade/tank)
    intake.hpp     # Intake with color sorting
    arm.hpp        # Arm with state machine (pickup/score/idle)
    clamp.hpp      # Mobile goal clamp pneumatic
    doinker.hpp    # Doinker mechanism
    selector.hpp   # Autonomous selector on brain screen
    distance_align.hpp  # Wall alignment using distance sensors
  globals.hpp      # Port definitions, externs for all subsystems/hardware
  pros/            # PROS API headers (read-only)
  lemlib/          # LemLib headers (read-only)
  okapi/           # OkapiLib headers (read-only)
  display/         # LVGL display library (read-only)

src/
  main.cpp         # Entry points: initialize(), autonomous(), opcontrol()
  globals.cpp      # Global object instantiation (motors, sensors, subsystems)
  subsystems/      # Subsystem implementations (.cpp files)

firmware/          # Pre-compiled libraries (.a files) - DO NOT MODIFY
bin/               # Build outputs
.d/                # Dependency files (auto-generated)
```

### Hardware Configuration (globals.hpp)

**Motor ports** are defined as macros at top of `globals.hpp`:
- Drivetrain: 6 motors (3 left, 3 right) with blue gearset (600 RPM)
- Intake: 3 motors (ports 18, 19, 20) - mixed gearsets for different functions
- Arm: 2 motors (both port 22 currently - likely needs update)

**Sensors:**
- IMU (port 10) - inertial measurement for heading
- Rotation sensors (port 22) - odometry tracking wheels
- Optical sensor (port 22) - color detection for intake sorting
- Distance sensors (port 22) - wall alignment

**Pneumatics (ADI ports):**
- Clamp: 'C'
- Doinker: 'H'
- Intake piston: 'B'
- Arm piston: 'D'

**IMPORTANT:** Many sensors share port 22 - this is a configuration error. When modifying hardware, update port numbers in `globals.hpp` FIRST, then update `globals.cpp` instantiation if needed.

### Subsystems Overview

All subsystems live in `namespace subsystems`. Key classes:

1. **Movement** (`movement.hpp/cpp`)
   - Simple wrapper around LemLib chassis
   - `arcade_drive(leftY, rightX)` - driver control

2. **Auton** (`auton.hpp/cpp`)
   - Contains all autonomous routines
   - Helper methods: `go_forward()`, `go_backward()` - move relative to current heading
   - Pre-programmed routines: `elimSAFERED/BLUE()`, `swpRED/BLUE()`, `skills()`
   - Uses `chassis->moveToPoint()` extensively with field coordinates

3. **Intake** (`intake.hpp/cpp`)
   - Color sorting capability (DONUT_COLOR::RED or BLUE)
   - Task-based async operation: `startColourSort()`, `deactivate()`
   - Optical sensor reads hue to reject opponent colors
   - Has pneumatic piston for raising/lowering

4. **Arm** (`arm.hpp/cpp`)
   - State machine with positions: PICKUP, IDLE, READY, SCORE_MOVE, ALLIANCE_STAKE
   - Motor positions are in encoder ticks
   - Methods: `goToPickup()`, `goToIdle()`, `readyScore()`, `score()`

5. **Clamp** (`clamp.hpp/cpp`)
   - Pneumatic control for mobile goal
   - `clamp_stake()`, `declamp_stake()`

6. **Selector** (`selector.hpp/cpp`)
   - Brain LCD UI for selecting autonomous routines
   - Runs in `competition_initialize()`

7. **DistanceAlign** (`distance_align.hpp/cpp`)
   - Wall alignment using distance sensors
   - `calculateAngleOneWall()`, `calculateDistOneWall()`

### Autonomous Development Patterns

When writing autonomous routines in `Auton` class:

1. **Set starting pose** at beginning:
   ```cpp
   chassis->setPose(x_inches, y_inches, heading_degrees);
   ```

2. **Use field coordinates** (not relative movements when possible):
   ```cpp
   chassis->moveToPoint(24.0, -36.0, 2000, {.maxSpeed = 100});
   chassis->waitUntil(5);  // continue when within 5 inches
   ```

3. **Coordinate subsystem actions** with motion:
   ```cpp
   intake.startColourSort();  // Start intake task
   chassis->moveToPoint(...);  // Move while intaking
   pros::delay(500);           // Allow time for intake
   intake.deactivate();        // Stop intake
   ```

4. **Use helpers** for heading-relative movement:
   ```cpp
   go_forward(12.0, 1000);  // Move 12" forward relative to current heading
   ```

5. **Alliance mirroring:** Negate X coordinates and headings for RED↔BLUE
   - RED example: `chassis->setPose(-9, -46.1, 150);`
   - BLUE example: `chassis->setPose(9, -46.1, -150);`

### PID Tuning

Motion control is tuned in `globals.cpp`:

```cpp
// Lateral (straight line) PID
lemlib::ControllerSettings lateralPID(
    10,  // kP - proportional gain
    0,   // kI - integral gain
    1,   // kD - derivative gain
    3,   // anti-windup
    1,   // small error range (inches)
    100, // small error timeout (ms)
    3,   // large error range (inches)
    500, // large error timeout (ms)
    20   // max acceleration (slew)
);

// Angular (turning) PID - similar structure
lemlib::ControllerSettings angularPID(2.2, 0, 10, ...);
```

**To tune:** Adjust kP/kI/kD values. Higher kP = more aggressive, higher kD = more damping.

### Entry Points (main.cpp)

- `initialize()` - Runs on robot boot. Calibrates IMU, sets brake modes
- `competition_initialize()` - Runs when competition switch connected. Shows auton selector
- `autonomous()` - 15-second autonomous period. Call `auton.run_auton(...)` or specific routine
- `opcontrol()` - Driver control period. Runs control loops for all subsystems

### Common Gotchas

1. **LemLib requires calibration:** Always call `chassis.calibrate()` in `initialize()` before using odometry
2. **Async tasks:** Intake uses PROS tasks. Don't forget to call `deactivate()` or `endIntakeTask()`
3. **Blocking vs non-blocking:** Most `chassis` methods are non-blocking. Use `waitUntilDone()` or `waitUntil(dist)` to synchronize
4. **Units:** LemLib uses inches and degrees. Motor encoders use ticks
5. **Port conflicts:** Multiple sensors on port 22 - fix before competition
6. **Brake modes:** Set in `initialize()` and `opcontrol()` - different modes for auton vs driver

## LemLib-Specific Notes

- **DO NOT MODIFY** LemLib library files in `include/lemlib/` or `firmware/LemLib.a`
- LemLib handles odometry fusion from IMU and tracking wheels automatically
- Drivetrain config in `globals.cpp`: 9.9" track width, 3.25" omni wheels, 450 RPM, 2° horizontal drift
- Current config: odometry sensors set to `nullptr` - using IMU-only tracking (not ideal for competition)
- **Recommended:** Wire tracking wheels to rotation sensor ports and update `sensors` initialization

## Testing Workflow

1. Make changes to subsystem code
2. Build: `make clean && make`
3. Upload: `pros upload` (or use PROS GUI)
4. Test on robot in driver control first
5. Test autonomous routines by calling them from `opcontrol()` with button press, OR
6. Use competition switch / field control to trigger `autonomous()`

## Common Development Tasks

**Add new autonomous routine:**
1. Add enum value to `AUTON_ROUTINE` in `robot/auton.hpp`
2. Declare method in `Auton` class
3. Implement in `src/subsystems/auton.cpp`
4. Add case to `run_auton()` switch statement
5. Update selector UI in `selector.cpp` if needed

**Add new subsystem:**
1. Create header in `include/robot/yoursubsystem.hpp`
2. Create implementation in `src/subsystems/yoursubsystem.cpp`
3. Add `#include` and extern declaration in `globals.hpp`
4. Instantiate global object in `globals.cpp`
5. Call methods from `opcontrol()` or autonomous routines

**Change motor/sensor port:**
1. Update `#define` in `globals.hpp`
2. Rebuild - no code changes needed (uses macros)

---

## 🚀 Universal Framework (Game-Agnostic Components)

This codebase has been refactored into a **universal framework** that can be reused across multiple VEX game seasons with minimal changes. The framework achieves **80%+ code reuse** between seasons.

### Framework Philosophy: "Simple > Complex"

**Core Principle:** Provide simple, copyable components over complex inheritance hierarchies.

**Why:** High school students need to:
- Understand code quickly
- Copy-paste for new mechanisms
- Modify without breaking the framework

### Framework Structure

```
lib/                    # Universal, game-agnostic components
  Pneumatic.hpp/.cpp    # Generic pneumatic control
  MotorSubsystem.hpp/.cpp  # Base class for all motor subsystems

robot/                  # Game-specific subsystems (High Stakes)
  intake.hpp/.cpp       # Extends lib::MotorSubsystem
  arm.hpp/.cpp         # Extends lib::MotorSubsystem

robot_config.hpp/.cpp   # Centralized initialization system
globals.hpp/.cpp        # Hardware configuration
templates/              # Copy-paste examples for students
  pneumatic_template.cpp
  motor_subsystem_template.cpp
  NEW_SEASON_SETUP.md

MIGRATION_CHECKLIST.md  # Printable checklist for new seasons
CONTEXT.md             # Framework documentation and design decisions
```

---

## lib::Pneumatic - Universal Pneumatic Component

**Purpose:** ONE class for ANY pneumatic mechanism in ANY game.

### Usage

```cpp
// In globals.hpp:
#define MECHANISM_PORT 'A'
extern lib::Pneumatic mechanism;

// In globals.cpp:
#include "lib/Pneumatic.hpp"
lib::Pneumatic mechanism(MECHANISM_PORT);

// In main.cpp opcontrol():
mechanism.run(DIGITAL_Y);  // Toggle on button press
```

### Available Methods

```cpp
lib::Pneumatic mechanism(port);
lib::Pneumatic mechanism(port, true);  // Start extended

// Toggle
mechanism.toggle();

// Extend (all do same thing - use what reads best)
mechanism.extend();
mechanism.activate();
mechanism.open();
mechanism.deploy();

// Retract (all do same thing)
mechanism.retract();
mechanism.deactivate();
mechanism.close();
mechanism.stow();

// Manual control
mechanism.set(true);        // Set to specific state
bool state = mechanism.getState();

// Controller integration
mechanism.run(DIGITAL_BUTTON);  // Toggle on new press (has debouncing)
```

### Game Examples

```cpp
// High Stakes
lib::Pneumatic clamp(CLAMP_PORT);      // Mobile goal clamp
lib::Pneumatic doinker(DOINKER_PORT);  // Doinker

// Future game examples
lib::Pneumatic wings(WINGS_PORT);      // Just rename!
lib::Pneumatic blocker(BLOCKER_PORT);
lib::Pneumatic lift(LIFT_PORT);
```

**Key Point:** Don't create new pneumatic classes! Just rename the variable.

---

## lib::MotorSubsystem - Universal Motor Base Class

**Purpose:** Base class for ALL motor-driven mechanisms. Eliminates duplicate motor control code.

### Direct Usage (Simple Mechanisms)

For basic motor control, use directly without creating a custom class:

```cpp
// In globals.cpp:
pros::Motor shooterMotor(SHOOTER_PORT, pros::E_MOTOR_GEARSET_06);
lib::MotorSubsystem shooter({shooterMotor});

// In main.cpp:
shooter.move(12000);          // Voltage control
shooter.stop();
```

### Inheritance (Complex Mechanisms)

For game-specific behavior, extend the base class:

```cpp
// In include/robot/mechanism.hpp:
#include "lib/MotorSubsystem.hpp"

class MyMechanism : public lib::MotorSubsystem {
public:
    MyMechanism(std::vector<pros::Motor> motors)
        : lib::MotorSubsystem(motors) {}

    // Use inherited methods:
    //   - move(int voltage)
    //   - moveAbsolute(float position, float speed)
    //   - moveRelative(float delta, float speed)
    //   - stop()
    //   - getPosition()
    //   - getVelocity()
    //   - getTemperature()
    //   - isMoving()
    //   - setBrakeMode(mode)
    //   - setZeroPosition(position)

    // Add game-specific methods:
    void customBehavior() {
        move(12000);  // Use base class method
    }

protected:
    // Access protected member: motors (pros::MotorGroup)
};
```

### Available Base Methods

```cpp
// Voltage control
void move(int voltage);  // -12000 to 12000 millivolts
void stop();

// Position control
void moveAbsolute(float position, float speed = 100);
void moveRelative(float delta, float speed = 100);

// State queries
float getPosition();      // Encoder ticks
float getVelocity();      // RPM
float getTemperature();   // Celsius
bool isMoving();          // velocity > threshold

// Configuration
void setBrakeMode(pros::motor_brake_mode_e mode);
void setZeroPosition(float position = 0);
```

### Examples in Codebase

**Intake** (extends lib::MotorSubsystem):
- Inherits: basic motor control
- Adds: color sorting, limit switches, optical sensor

**Arm** (extends lib::MotorSubsystem):
- Inherits: basic motor control
- Adds: position presets, state machine, pneumatic integration

---

## robot_config - Centralized Initialization

**Purpose:** Single function to initialize entire robot.

### Usage

```cpp
// In main.cpp initialize():
#include "robot_config.hpp"

void initialize() {
    pros::lcd::initialize();

    robot_config::initialize();  // Setup EVERYTHING

    chassis.setPose(0, 0, 0);    // Game-specific setup
    // ...
}
```

### What It Does

1. Controller initialization
2. Drivetrain motor logging
3. Sensor initialization (IMU, rotation sensors)
4. **IMU calibration with timeout**
5. LemLib chassis calibration
6. Subsystem status logging
7. Comprehensive console output

### IMU Calibration

```cpp
// Automatic calibration with error handling:
robot_config::calibrate_imu();  // Called by initialize()

// Features:
// - Timeout after 3 seconds
// - Status messages on controller
// - Console logging
// - Error detection
```

---

## Migration to New Season

**Time Required:** 1-2 hours (vs. days with old codebase!)

### Quick Start

1. **Read:** `templates/NEW_SEASON_SETUP.md` (comprehensive guide)
2. **Print:** `MIGRATION_CHECKLIST.md` (step-by-step checklist)
3. **Reference:** `templates/` directory (copy-paste examples)

### 6-Step Process

1. **Update Ports** (15 min) - `globals.hpp`
2. **Rename Pneumatics** (10 min) - `globals.cpp` & `globals.hpp`
3. **Create/Update Subsystems** (30 min) - Use templates
4. **Autonomous** (20 min) - `auton.cpp`
5. **Driver Control** (15 min) - `main.cpp`
6. **Test & Document** (20 min)

### What NOT to Change

**NEVER modify these (universal components):**
- ❌ `lib/Pneumatic.hpp` and `.cpp`
- ❌ `lib/MotorSubsystem.hpp` and `.cpp`
- ❌ `robot_config.hpp` and `.cpp`
- ❌ LemLib/PROS/OkapiLib libraries

**Only update:**
- ✅ Port definitions (`globals.hpp`)
- ✅ Hardware instantiation (`globals.cpp`)
- ✅ Autonomous routines (`auton.cpp`)
- ✅ Driver control (`main.cpp`)
- ✅ Game-specific subsystems (`robot/`)

---

## Templates & Examples

### templates/pneumatic_template.cpp
- 3 complete examples (clamp, wings, blocker)
- All available methods documented
- Autonomous usage
- Troubleshooting guide

### templates/motor_subsystem_template.cpp
- Example 1: Simple flywheel (minimal extension)
- Example 2: Color sorting intake (complex)
- Example 3: Lift with presets (moderate)
- Implementation patterns
- Usage examples

### templates/NEW_SEASON_SETUP.md
- Comprehensive 6-step guide
- Code examples for each step
- Common mistakes to avoid
- Testing checklist

### MIGRATION_CHECKLIST.md
- Printable checkbox format
- 8 phases with time estimates
- Troubleshooting section
- Success criteria

---

## Code Quality Standards

### Naming Conventions

**Generic Components (lib/):**
```cpp
lib::Pneumatic        // NOT lib::Clamp
lib::MotorSubsystem   // NOT lib::Intake
```

**Student Variables:**
```cpp
// Students rename for their game:
lib::Pneumatic clamp(PORT);      // High Stakes
lib::Pneumatic wings(PORT);      // Future game
```

### Documentation Requirements

Every public method MUST have:
1. Brief description
2. Parameter documentation
3. Usage example (when non-obvious)

```cpp
/**
 * @brief Move intake forward at specified speed
 * @param speed (0-1) multiplier for speed, defaults to full
 */
void move_forward(float speed = 1);
```

### File Organization

```
include/lib/      # Generic components
include/robot/    # Game-specific
src/lib/          # Generic implementations
src/subsystems/   # Game-specific implementations
templates/        # Student examples
```

---

## Framework Metrics

### Week 1: Pneumatic Component
- ✅ Eliminated 116 lines of duplicate code
- ✅ Clamp + Doinker → single `lib::Pneumatic` class
- ✅ 93 lines of comprehensive documentation

### Week 2: Motor Subsystem
- ✅ Eliminated 75 lines of duplicate code
- ✅ Created universal motor base class
- ✅ Intake & Arm now extend base class

### Week 3: Configuration System
- ✅ Centralized initialization (150+ lines)
- ✅ Reorganized globals.cpp (7 clear sections)
- ✅ IMU calibration with error handling

### Week 4: Templates & Documentation
- ✅ 400+ lines of student templates
- ✅ Printable migration checklist
- ✅ Comprehensive documentation

### Overall Achievement
- ✅ **80%+ code reuse** between seasons
- ✅ **1-2 hour setup time** (from days)
- ✅ **Zero functionality regressions**
- ✅ **Student-friendly** design

---

## Advanced Topics

### Custom Subsystem Pattern

1. **Decide:** Do you need a custom class?
   - **No:** Use `lib::MotorSubsystem` or `lib::Pneumatic` directly
   - **Yes:** Extend `lib::MotorSubsystem`

2. **Create header:** `include/robot/mechanism.hpp`
   ```cpp
   class Mechanism : public lib::MotorSubsystem {
       // Use inherited methods
       // Add game-specific behavior
   };
   ```

3. **Implement:** `src/subsystems/mechanism.cpp`

4. **Instantiate:** `globals.cpp`

5. **Use:** `main.cpp` or autonomous

### PID Tuning (LemLib)

Located in `globals.cpp`:
```cpp
// Lateral PID (forward/backward)
lemlib::ControllerSettings lateralPID(
    10,   // kP - aggression
    0,    // kI - steady-state
    1,    // kD - damping
    // ... other params
);

// Angular PID (turning)
lemlib::ControllerSettings angularPID(
    2.2,  // kP
    0,    // kI
    10,   // kD
    // ...
);
```

**Tuning Process:**
1. Start with kP only (kI=0, kD=0)
2. Increase kP until oscillation
3. Reduce kP by 30%
4. Add kD to reduce overshoot
5. Add kI only if needed for steady-state

### Autonomous Development Pattern

```cpp
// In auton.cpp:
void Auton::myRoutine() {
    // Use universal components (auto-renamed)
    wings.extend();

    // Use LemLib
    chassis.setPose(0, 0, 0);
    chassis.moveToPoint(24, 24, 2000);
    chassis.waitUntilDone();

    // Use custom subsystems
    shooter.spinUp();
    pros::delay(500);

    wings.retract();
}
```

---

## Troubleshooting Framework Issues

### Problem: Pneumatic doesn't work
- Check port definition in `globals.hpp`
- Verify instantiation in `globals.cpp`
- Check extern declaration in `globals.hpp`
- Test with opposite method (extend ↔ retract)

### Problem: Motor subsystem won't compile
- Ensure `#include "lib/MotorSubsystem.hpp"`
- Verify inheritance: `: public lib::MotorSubsystem`
- Check constructor passes motors to base: `: lib::MotorSubsystem(motors)`

### Problem: Can't find base class methods
- Use `this->` prefix: `this->move(12000)`
- Or reference directly: `move(12000)` (should work)
- Check you're calling from derived class method

---

## Resources

- **CONTEXT.md** - Framework design decisions and architecture
- **templates/** - Copy-paste examples
- **MIGRATION_CHECKLIST.md** - New season setup
- **This file (CLAUDE.md)** - Complete development guide

---

*Framework Version: 1.0*
*Last Updated: 2025-10-07*
*Team: 839Y*
