# Robot Simulation System

Test autonomous routines **without physical robot** using mock sensor data and virtual physics.

## Features

- ✅ **Virtual chassis** - Simulates LemLib movement commands
- ✅ **Mock sensors** - IMU heading, motor temps/currents, battery voltage
- ✅ **Field visualization** - ASCII art showing robot position on 144" x 144" field
- ✅ **Deterministic physics** - Reproducible for debugging
- ✅ **Telemetry compatible** - Works with existing telemetry system

## Quick Start

### 1. Enable Simulation Mode

In `src/main.cpp`, uncomment these lines in `initialize()`:

```cpp
// ========================================================================
// SIMULATION MODE
// ========================================================================
simulation.setEnabled(true);  // Enable simulation
simulation.reset();            // Reset to starting position
```

### 2. Run Autonomous

Your autonomous routines will now use simulated data instead of real hardware:

```cpp
void autonomous() {
    // This now runs in simulation!
    auton.odomDriveTest();

    // At the end, see the results:
    simulation.printState();
    simulation.drawField();
}
```

### 3. View Results

**Console Output:**
```
[Simulation] ENABLED - Using mock sensor data
[Simulation] Reset to X:0.0 Y:0.0 H:0.0

[Sim] X:48.0 Y:0.1 H:-1.2 | Vel:0.0 in/s | Motors: 42C 1200mA | Battery: 12.45V

========== FIELD VISUALIZATION ==========
  Field: 144" x 144" | Robot: (48.0, 0.1) @ -1°

##############################
#                            #
#                            #
#     .    .    .    .       #
#                            #
#                            #
#     .    .    .    .       #
#                            #
#          R  +              #  ← Robot position
#     .    .    .    .       #
#                            #
##############################
=========================================
```

## Calibration

The simulation has been **calibrated with real robot data**:

**Real Robot Test:**
- Distance: 48 inches
- Time: 1.8 seconds
- Speed setting: 60% (maxSpeed)

**Simulation Parameters:**
- Max velocity: 75 in/s (at 100% speed)
- Linear acceleration: 50 in/s²
- Trapezoidal velocity profile (accel → constant → decel)

### Validate Calibration

Test simulation accuracy against your robot:

```cpp
void autonomous() {
    simulation.setEnabled(true);
    simulation.validateCalibration();
}
```

**Output:**
```
========== SIMULATION CALIBRATION TEST ==========
Real robot data: 48" in 1.8s @ 60% speed

Simulating 48" drive...

--- RESULTS ---
Simulated time: 1.82 seconds
Real robot time: 1.8 seconds
Error: 0.02 seconds (1.1%)
Final position: X:48.0 Y:0.0 (target: 48.0, 0.0)
✓ CALIBRATION GOOD (within 0.3s)
=================================================
```

If your robot is different, adjust `LINEAR_ACCEL` in `include/robot/simulation.hpp`.

## How It Works

### Virtual Physics

The simulation tracks:
- **Position** (X, Y in inches)
- **Heading** (theta in degrees)
- **Velocity** (linear and angular)
- **Acceleration** (realistic limits)

When you call `chassis.moveToPoint(x, y)`, the simulation:
1. Calculates distance and angle to target
2. Accelerates virtual robot (max 40 in/s²)
3. Updates position based on velocity
4. Updates chassis pose so LemLib thinks it moved

### Mock Sensor Data

**IMU:**
- Returns virtual heading (updated by physics)
- No drift (perfect simulation)

**Motors:**
- Temperature: 35°C idle → 55°C max during movement
- Current: 500mA idle → 2000mA at full speed
- Realistic heating/cooling rates

**Battery:**
- Starts at 12.6V
- Drains ~10mV per second
- Minimum 11.0V

### Integration with Existing Systems

**Telemetry:** Logs simulated data to CSV (same format as real robot)

**Alerts:** Monitors simulated temps, currents, battery

**IMU Drift:** Disabled during simulation (perfect IMU)

## API Reference

### Enable/Disable

```cpp
simulation.setEnabled(true);   // Use mock data
simulation.setEnabled(false);  // Use real hardware
```

### Reset

```cpp
simulation.reset();  // Reset to chassis starting pose
```

### Manual Updates

```cpp
// Called automatically in background task
simulation.update();  // Update physics (10ms time step)
```

### Visualization

```cpp
simulation.printState();  // Print X, Y, heading, velocity, sensors
simulation.drawField();   // ASCII field map
```

### Mock Sensor Getters

```cpp
float heading = simulation.getMockHeading();
float temp = simulation.getMockMotorTemp(0);  // Motor 0-5
float current = simulation.getMockMotorCurrent(0);
uint32_t battery = simulation.getMockBatteryVoltage();  // mV
```

## Use Cases

### 1. Develop Autonomous Without Robot

**Problem:** Robot is at competition, but you want to code new routes

**Solution:**
```cpp
// In initialize():
simulation.setEnabled(true);

// In autonomous():
chassis.setPose(-36, -36, 45);  // Red corner start
chassis.moveToPoint(0, 0, 2000);  // Drive to center
chassis.waitUntilDone();

simulation.drawField();  // See where robot ended up
```

### 2. Debug Path Planning

**Problem:** Route isn't working, but you can't see why

**Solution:**
```cpp
void autonomous() {
    // Print state at each waypoint
    chassis.moveToPoint(24, 0, 2000);
    chassis.waitUntilDone();
    simulation.printState();  // Check position

    chassis.turnToHeading(90, 1000);
    chassis.waitUntilDone();
    simulation.printState();  // Check heading
}
```

### 3. Test Telemetry System

**Problem:** Want to verify telemetry logging works before competition

**Solution:**
```cpp
simulation.setEnabled(true);
telemetry.init();

// Run full autonomous
auton.template_simple_2ball();

telemetry.close();
// Now analyze telemetry CSV with simulated data
```

### 4. Validate Autonomous Templates

**Problem:** Students wrote autonomous, need to check it compiles and runs

**Solution:**
```cpp
// Enable simulation
simulation.setEnabled(true);

// Test each template
auton.template_simple_2ball();
simulation.printState();

auton.template_4ball_checkpoints();
simulation.printState();

// All templates run without robot!
```

## Limitations

### What's Simulated

✅ Chassis position (X, Y, heading)
✅ Motor temps and currents
✅ Battery voltage
✅ IMU heading
✅ Linear and angular velocity

### What's NOT Simulated

❌ Intake/arm subsystems (no mock)
❌ Pneumatics (no mock)
❌ Collision detection (robot can go through walls)
❌ Ball physics (no game elements)
❌ Sensor noise/drift (perfect sensors)
❌ Real motor stalling/overheating

### Accuracy

The simulation uses simplified physics:
- Constant acceleration (real motors vary)
- No wheel slip
- No momentum/inertia effects
- No field obstacles

**Use for:** Route planning, debugging movement logic, testing code structure

**Don't use for:** Precise PID tuning, collision avoidance, real competition predictions

## Advanced Usage

### Custom Physics Constants

Edit `include/robot/simulation.hpp`:

```cpp
static constexpr float MAX_LINEAR_VEL = 80.0;   // inches/sec
static constexpr float MAX_ANGULAR_VEL = 180.0; // degrees/sec
static constexpr float LINEAR_ACCEL = 40.0;     // in/s^2
static constexpr float ANGULAR_ACCEL = 360.0;   // deg/s^2
```

### Print State During Movement

Uncomment in `src/main.cpp`:

```cpp
void simulation_task(void* param) {
    while (true) {
        if (simulation.isEnabled()) {
            simulation.update();

            // Print every second (UNCOMMENT):
            static uint32_t lastPrint = 0;
            if (pros::millis() - lastPrint > 1000) {
                simulation.printState();
                lastPrint = pros::millis();
            }
        }
        pros::delay(10);
    }
}
```

### Programmatic Control

```cpp
// Get current state
lemlib::Pose pose = chassis.getPose();
// pose.x, pose.y, pose.theta from simulation

// Check if simulation active
if (simulation.isEnabled()) {
    // Simulation-specific code
}
```

## Troubleshooting

### Robot doesn't move in simulation

**Cause:** Physics update task not running

**Fix:** Check `autonomous()` starts simulation task:
```cpp
pros::Task simulation_bg(simulation_task, nullptr, "Simulation");
```

### Position doesn't match expectations

**Cause:** Acceleration limits prevent instant movement

**Fix:** Wait for movement to complete:
```cpp
chassis.moveToPoint(24, 24, 3000);  // Longer timeout
chassis.waitUntilDone();  // Block until arrived
```

### Simulation state not updating

**Cause:** `simulation.update()` not called

**Fix:** Ensure simulation task is running (check console for task start message)

## Examples

### Example 1: Simple Test Route

```cpp
void autonomous() {
    simulation.setEnabled(true);
    simulation.reset();

    // Square pattern
    chassis.setPose(0, 0, 0);

    chassis.moveToPoint(24, 0, 2000);
    chassis.waitUntilDone();

    chassis.moveToPoint(24, 24, 2000);
    chassis.waitUntilDone();

    chassis.moveToPoint(0, 24, 2000);
    chassis.waitUntilDone();

    chassis.moveToPoint(0, 0, 2000);
    chassis.waitUntilDone();

    simulation.drawField();
    // Should show robot back at origin
}
```

### Example 2: Competition Route Test

```cpp
void testCompetitionRoute() {
    simulation.setEnabled(true);

    // Red alliance starting position
    chassis.setPose(-60, -60, 45);

    // Drive to center
    chassis.moveToPoint(0, 0, 3000);
    chassis.waitUntilDone();
    simulation.printState();

    // Turn to face goal
    chassis.turnToHeading(90, 1000);
    chassis.waitUntilDone();
    simulation.printState();

    // Show final position on field
    simulation.drawField();
}
```

---

**Next Steps:**
1. Enable simulation in `initialize()`
2. Run autonomous routines
3. Check console for results
4. Iterate on route planning
5. Disable simulation before uploading to real robot
