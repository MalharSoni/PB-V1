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
