# Complete Odometry & PID Tuning Guide - Team 839Y

## Table of Contents
1. [Overview](#overview)
2. [Phase 1: Odometry Tuning](#phase-1-odometry-tuning)
3. [Phase 2: Lateral PID Tuning](#phase-2-lateral-pid-tuning)
4. [Phase 3: Angular PID Tuning](#phase-3-angular-pid-tuning)
5. [Phase 4: Fine-Tuning](#phase-4-fine-tuning)
6. [Troubleshooting](#troubleshooting)

---

## Overview

**Total Time:** 1-2 hours
**Required:** Robot, competition field or 12'x12' area, measuring tape

**Tuning Order (CRITICAL - Do NOT skip steps):**
1. ✅ Odometry (tracking accuracy)
2. ✅ Angular PID (turning)
3. ✅ Lateral PID (straight movement)
4. ✅ Fine-tuning combined movements

**Why This Order?**
- Without accurate odometry, you can't measure if PID is working
- Without good turning, you can't tune straight line movement
- Lateral PID depends on accurate heading from angular PID

---

## Phase 1: Odometry Tuning

### Goal
Robot should accurately track its position as it moves. After driving in a square, it should return to starting position with < 0.5" error.

### Current Configuration
Check `src/globals.cpp` lines 63-64:

```cpp
lemlib::TrackingWheel verticalTracking(&leftRotation, lemlib::Omniwheel::NEW_275, 0.0);
lemlib::TrackingWheel horizontalTracking(&rearRotation, lemlib::Omniwheel::NEW_275, -4.0);
```

**Parameters:**
- Wheel diameter: `NEW_275` = 2.75"
- Vertical offset: `0.0` inches (left tracking wheel)
- Horizontal offset: `-4.0` inches (rear tracking wheel)

### Step 1.1: Verify Wheel Diameters

**Measure your tracking wheels:**
1. Use calipers or ruler to measure wheel diameter
2. Common sizes:
   - `NEW_2` = 2.0"
   - `NEW_275` = 2.75"
   - `NEW_325` = 3.25"
   - `NEW_4` = 4.0"

**If diameter is wrong:**
```cpp
// Change in globals.cpp line 63-64
lemlib::TrackingWheel verticalTracking(&leftRotation, lemlib::Omniwheel::NEW_325, 0.0);
//                                                      ^^^^^^^^^^^ Update this
```

### Step 1.2: Measure Tracking Wheel Offsets

**Vertical Tracking Wheel (Left Side):**
1. Measure distance from wheel center to robot's center of rotation
2. If wheel is on the left: offset is NEGATIVE
3. If wheel is on the right: offset is POSITIVE
4. If centered: offset is 0.0

**Horizontal Tracking Wheel (Rear):**
1. Measure distance from wheel center to robot's center of rotation
2. If wheel is behind center: offset is NEGATIVE
3. If wheel is in front: offset is POSITIVE
4. If centered: offset is 0.0

**Example Measurement:**
```
Robot (top view):
  [Front]
    ___
   |   |
[L]|   |    L = Vertical tracking wheel, 2" left of center
   |   |        Offset = -2.0 (negative because left)
   |___|
     H       H = Horizontal tracking wheel, 4" behind center
               Offset = -4.0 (negative because rear)
  [Back]
```

**Update in globals.cpp:**
```cpp
lemlib::TrackingWheel verticalTracking(&leftRotation, lemlib::Omniwheel::NEW_275, -2.0);
//                                                                                ^^^^ Your measured offset
lemlib::TrackingWheel horizontalTracking(&rearRotation, lemlib::Omniwheel::NEW_275, -4.0);
//                                                                                  ^^^^ Your measured offset
```

### Step 1.3: Run Square Test

**Test Procedure:**
1. Build and upload: `make clean && make && pros upload`
2. Place robot on open field
3. Mark starting position with tape
4. Press "Autonomous" button
5. Robot drives in 24" x 24" square

**Check Results on Brain Screen:**
- Line 4: `Error: X.XX" H:X.X deg`
- Line 5: `Final: X:X.XX Y:X.XX H:X.X`
- Line 6: Live position (updates every 100ms)

**Target Results:**
- ✅ **Total error < 0.5 inches**
- ✅ **Heading error < 2 degrees**

**If error > 1 inch:**
→ Go to Step 1.4

**If error < 0.5 inches:**
→ Skip to Phase 2 (Angular PID)

### Step 1.4: Wheel Diameter Fine-Tuning

If square test shows consistent error, use this formula:

**Forward/Backward Drift (Y-axis error):**
```
New Vertical Diameter = Old Diameter × (Actual Distance / Measured Distance)

Example:
- Robot drove 24" forward but odometry shows 25"
- Old diameter: 2.75"
- New diameter = 2.75 × (24 / 25) = 2.64"

Update: lemlib::Omniwheel::NEW_275 → Use 2.64 custom value
```

**Left/Right Drift (X-axis error):**
```
New Horizontal Diameter = Old Diameter × (Actual Distance / Measured Distance)

Example:
- Robot should move 24" right but shows 23.5"
- Old diameter: 2.75"
- New diameter = 2.75 × (24 / 23.5) = 2.808"
```

**Custom Diameter Syntax:**
```cpp
// Instead of predefined sizes:
lemlib::TrackingWheel verticalTracking(&leftRotation, 2.64, -2.0);
//                                                     ^^^^ Custom diameter in inches
```

### Step 1.5: Offset Fine-Tuning

**If robot spins/rotates during straight movement:**

The tracking wheel offset might be wrong. Try adjusting in 0.25" increments:

```cpp
// If robot curves left during forward movement:
// Increase vertical offset magnitude (more negative if on left)
lemlib::TrackingWheel verticalTracking(&leftRotation, lemlib::Omniwheel::NEW_275, -2.5);
//                                                                                ^^^^ Was -2.0, now -2.5

// If robot curves right during forward movement:
// Decrease vertical offset magnitude
lemlib::TrackingWheel verticalTracking(&leftRotation, lemlib::Omniwheel::NEW_275, -1.5);
//                                                                                ^^^^ Was -2.0, now -1.5
```

### Step 1.6: IMU Calibration

**Ensure IMU is calibrated:**
1. Check console output during `initialize()`
2. Look for: `IMU calibration success!`
3. If fails: Check IMU port (should be port 10 in `globals.hpp`)

**During calibration:**
- Robot MUST be still
- Place on flat surface
- Don't touch robot for 3 seconds

---

## Phase 2: Angular PID Tuning

### Goal
Robot should turn smoothly to target heading without oscillation or overshooting.

### Current Configuration
Check `src/globals.cpp` lines 131-141:

```cpp
lemlib::ControllerSettings angularPID(
    2.2,    // kP - Proportional gain
    0,      // kI - Integral gain
    10,     // kD - Derivative gain
    3,      // Anti-windup range
    1,      // Small error range (degrees)
    100,    // Small error timeout (ms)
    3,      // Large error range (degrees)
    500,    // Large error timeout (ms)
    0       // Maximum acceleration (slew)
);
```

### Understanding PID Parameters

**kP (Proportional):** "How aggressive?"
- Higher kP = faster turning, more aggressive
- Too high = oscillation (robot shakes back and forth)
- Too low = slow, won't reach target

**kD (Derivative):** "How smooth?"
- Higher kD = smoother, less overshoot
- Too high = sluggish, takes forever
- Too low = overshoots target

**kI (Integral):** "Long-term correction"
- Usually keep at 0 for turning
- Only increase if robot consistently stops 2-3° short

### Step 2.1: Test Current Turning

**Create test routine in `src/auton.cpp`:**

Add this function after `odomSquareTest()`:

```cpp
void Auton::turnTest() {
    chassis->setPose(0, 0, 0);
    pros::delay(1000);

    // Test 90° turns in all directions
    chassis->turnToHeading(90, 2000);   // Turn left 90°
    chassis->waitUntilDone();
    pros::delay(1000);

    chassis->turnToHeading(180, 2000);  // Turn to 180°
    chassis->waitUntilDone();
    pros::delay(1000);

    chassis->turnToHeading(270, 2000);  // Turn to 270°
    chassis->waitUntilDone();
    pros::delay(1000);

    chassis->turnToHeading(0, 2000);    // Return to 0°
    chassis->waitUntilDone();
    pros::delay(1000);

    // Test 180° turn
    chassis->turnToHeading(180, 2000);
    chassis->waitUntilDone();

    // Display final heading
    lemlib::Pose pose = chassis->getPose();
    pros::lcd::print(5, "Final heading: %.2f", pose.theta);
}
```

**Add to header (`include/robot/auton.hpp`):**
```cpp
void turnTest();  // Add after odomSquareTest() declaration
```

**Run in autonomous (`src/main.cpp`):**
```cpp
void autonomous() {
    auton.turnTest();
}
```

### Step 2.2: Tune kP (Start Here)

**Watch the robot turn:**

**Problem: Robot oscillates/shakes at target**
- kP is too high
- Reduce by 20-30%
- Example: `2.2` → `1.5`

**Problem: Robot turns slowly, doesn't reach target**
- kP is too low
- Increase by 20-30%
- Example: `2.2` → `3.0`

**Problem: Robot overshoots and comes back**
- kP might be okay, move to kD tuning

**Target Behavior:**
- Robot turns smoothly
- Slows down as it approaches target
- Settles quickly without oscillation

### Step 2.3: Tune kD (Smoothing)

**After kP is roughly tuned:**

**Problem: Robot overshoots target by 5-10°**
- kD is too low
- Increase by 2-5
- Example: `10` → `15`

**Problem: Robot takes forever to turn**
- kD is too high
- Decrease by 2-5
- Example: `10` → `5`

**Target Behavior:**
- Minimal overshoot (< 2°)
- Reaches target in reasonable time

### Step 2.4: Tune kI (Rarely Needed)

**Only adjust if:**
- Robot consistently stops 2-3° short of target
- Already tried increasing kP

**Start small:**
- Add `kI = 0.01` or `0.05`
- Test, increase gradually if needed

**Warning:** Too much kI causes "integral windup" = robot spins wildly

### Step 2.5: Tuning Process Example

**Iteration 1:** Test with default values
```cpp
angularPID(2.2, 0, 10, ...)  // Starting point
```
**Result:** Robot oscillates ±5° at target
**Action:** Reduce kP

**Iteration 2:**
```cpp
angularPID(1.5, 0, 10, ...)  // Reduced kP
```
**Result:** Better, but still overshoots by 8°
**Action:** Increase kD

**Iteration 3:**
```cpp
angularPID(1.5, 0, 15, ...)  // Increased kD
```
**Result:** Overshoots reduced to 2°, settling time good
**Action:** Slight kP increase for speed

**Final:**
```cpp
angularPID(1.8, 0, 15, ...)  // Tuned values
```
**Result:** ✅ Fast, smooth, minimal overshoot

---

## Phase 3: Lateral PID Tuning

### Goal
Robot drives straight to target coordinates smoothly without oscillation.

### Current Configuration
Check `src/globals.cpp` lines 118-128:

```cpp
lemlib::ControllerSettings lateralPID(
    10,     // kP - Proportional gain
    0,      // kI - Integral gain
    1,      // kD - Derivative gain
    3,      // Anti-windup range
    1,      // Small error range (inches)
    100,    // Small error timeout (ms)
    3,      // Large error range (inches)
    500,    // Large error timeout (ms)
    20      // Maximum acceleration (slew)
);
```

### Step 3.1: Test Current Lateral Movement

**Create test routine:**

Add to `src/auton.cpp` after `turnTest()`:

```cpp
void Auton::straightTest() {
    chassis->setPose(0, 0, 0);
    pros::delay(1000);

    // Forward movement tests
    chassis->moveToPoint(0, 24, 3000);  // 24" forward
    chassis->waitUntilDone();
    pros::delay(1000);

    chassis->moveToPoint(0, 48, 3000);  // Another 24" forward
    chassis->waitUntilDone();
    pros::delay(1000);

    // Backward movement
    chassis->moveToPoint(0, 24, 3000);  // Back 24"
    chassis->waitUntilDone();
    pros::delay(1000);

    chassis->moveToPoint(0, 0, 3000);   // Return to start
    chassis->waitUntilDone();

    // Display final position
    lemlib::Pose pose = chassis->getPose();
    pros::lcd::print(5, "Final: X:%.2f Y:%.2f", pose.x, pose.y);

    float error = sqrt(pose.x * pose.x + pose.y * pose.y);
    pros::lcd::print(4, "Error: %.2f inches", error);
}
```

### Step 3.2: Tune Lateral kP

**Watch the robot drive:**

**Problem: Robot jerks/stutters during acceleration**
- kP is too high
- Reduce by 20-30%
- Example: `10` → `7`

**Problem: Robot moves slowly, takes forever**
- kP is too low
- Increase by 20-30%
- Example: `10` → `13`

**Problem: Robot oscillates side-to-side while driving**
- kP is too high OR angular PID needs tuning
- Try reducing kP first
- If persists, revisit angular PID

**Target Behavior:**
- Smooth acceleration
- Reaches target in reasonable time
- No side-to-side wiggling

### Step 3.3: Tune Lateral kD

**After kP is tuned:**

**Problem: Robot overshoots target by several inches**
- kD is too low
- Increase by 5-10
- Example: `1` → `10`

**Problem: Robot decelerates way too early, crawls to target**
- kD is too high
- Decrease by 2-5
- Example: `10` → `5`

**Target Behavior:**
- Decelerates smoothly near target
- Stops within 1" of target point

### Step 3.4: Tune Slew Rate (Acceleration Limit)

**Last parameter in lateralPID:** `20` = max acceleration

**Problem: Robot spins out / wheels slip on acceleration**
- Slew rate too high
- Reduce by 5-10
- Example: `20` → `10`

**Problem: Robot accelerates too slowly**
- Slew rate too low
- Increase by 5
- Example: `20` → `30`

**Note:** Higher = faster acceleration, but more wheel slip risk

---

## Phase 4: Fine-Tuning Combined Movements

### Goal
Ensure PID works well when combining turning and driving.

### Step 4.1: Combined Movement Test

**Create complex test:**

```cpp
void Auton::complexTest() {
    chassis->setPose(0, 0, 0);
    pros::delay(1000);

    // Move and turn sequence
    chassis->moveToPoint(24, 24, 3000);      // Drive to (24, 24)
    chassis->waitUntilDone();
    pros::delay(500);

    chassis->turnToHeading(180, 2000);       // Turn around
    chassis->waitUntilDone();
    pros::delay(500);

    chassis->moveToPoint(0, 24, 3000);       // Drive to (0, 24)
    chassis->waitUntilDone();
    pros::delay(500);

    chassis->turnToPoint(0, 0, 2000);        // Turn to face origin
    chassis->waitUntilDone();
    pros::delay(500);

    chassis->moveToPoint(0, 0, 3000, {.forwards = false});  // Drive backward to start
    chassis->waitUntilDone();

    // Check accuracy
    lemlib::Pose pose = chassis->getPose();
    float error = sqrt(pose.x * pose.x + pose.y * pose.y);
    pros::lcd::print(4, "Total error: %.2f\"", error);
}
```

### Step 4.2: Test Real Autonomous Paths

**Use your actual match autonomous:**
1. Run full autonomous routine
2. Check if robot reaches all waypoints accurately
3. Note any specific movements that fail

**Common Issues:**

**Problem: Robot accurate in tests but drifts in real autonomous**
- Paths are too long without correction
- Add intermediate waypoints every 36-48 inches

**Problem: Robot misses targets when moving backwards**
- Backwards movement might need different PID
- LemLib doesn't support separate PIDs, so:
  - Option 1: Use `.maxSpeed` to slow down backwards movements
  - Option 2: Avoid long backwards paths

---

## Quick Reference: PID Values

### Starting Values (Your Current Config)
```cpp
// Angular PID (Turning)
angularPID(2.2, 0, 10, 3, 1, 100, 3, 500, 0)

// Lateral PID (Driving)
lateralPID(10, 0, 1, 3, 1, 100, 3, 500, 20)
```

### Conservative Values (Good Starting Point)
```cpp
// Angular PID - Smooth, slower turns
angularPID(2.0, 0, 12, 3, 1, 100, 3, 500, 0)

// Lateral PID - Smooth, controlled movement
lateralPID(8, 0, 5, 3, 1, 100, 3, 500, 15)
```

### Aggressive Values (Fast Competition)
```cpp
// Angular PID - Fast, precise turns
angularPID(3.5, 0, 18, 3, 1, 100, 3, 500, 0)

// Lateral PID - Fast acceleration
lateralPID(15, 0, 3, 3, 1, 100, 3, 500, 30)
```

---

## Troubleshooting

### Problem: Robot doesn't move at all
**Check:**
1. Motors plugged in and powered
2. Motor ports correct in `globals.hpp`
3. Brake mode set to `BRAKE` not `HOLD`

### Problem: Robot moves but tracking doesn't update
**Check:**
1. Tracking wheel sensors connected (ports 1 and 2)
2. Tracking wheels spin freely (not stuck)
3. `OdomSensors` has `&verticalTracking` and `&horizontalTracking` (not nullptr)

### Problem: Position drifts over time
**Cause:** Odometry wheel diameter or offset wrong
**Fix:** Rerun Phase 1 (Odometry Tuning)

### Problem: Robot oscillates constantly
**Cause:** kP too high or kD too low
**Fix:**
1. Reduce kP by 30%
2. Increase kD by 50%
3. Retest

### Problem: Robot turns wrong direction
**Cause:** IMU or motor directions reversed
**Fix:**
1. Check motor direction settings
2. Try reversing motor groups in `globals.cpp`

### Problem: One test works but another fails
**Cause:** PID might be over-tuned for one scenario
**Fix:**
1. Find middle-ground values
2. Test with various distances and angles
3. Tune for average performance across all tests

---

## Tuning Checklist

### ✅ Phase 1: Odometry (30-45 min)
- [ ] Measure and set correct wheel diameters
- [ ] Measure and set tracking wheel offsets
- [ ] Run square test
- [ ] Achieve < 0.5" position error
- [ ] Achieve < 2° heading error

### ✅ Phase 2: Angular PID (20-30 min)
- [ ] Create `turnTest()` function
- [ ] Tune kP for smooth turning
- [ ] Tune kD to reduce overshoot
- [ ] Test 90°, 180°, 270° turns
- [ ] Achieve < 2° final error

### ✅ Phase 3: Lateral PID (20-30 min)
- [ ] Create `straightTest()` function
- [ ] Tune kP for smooth acceleration
- [ ] Tune kD for smooth deceleration
- [ ] Test forward and backward movement
- [ ] Achieve < 1" stopping error

### ✅ Phase 4: Combined Testing (15-20 min)
- [ ] Create `complexTest()` function
- [ ] Test turn + drive combinations
- [ ] Run actual autonomous routine
- [ ] Fine-tune any problem movements

---

## Final Tips

1. **Change ONE parameter at a time**
   - Don't adjust kP and kD simultaneously
   - Build and upload after each change
   - Document what you changed

2. **Test on competition field or similar surface**
   - PID behaves differently on carpet vs. foam tiles
   - Tune on the surface you'll compete on

3. **Battery matters**
   - Fresh battery = more power = different PID behavior
   - Test with battery at ~60-80% charge

4. **Save your tuned values**
   - Document final PID values in this file
   - Commit to git after successful tuning
   - Don't retune before every match

5. **When in doubt, go conservative**
   - Slower, smoother movements are more reliable
   - Speed can be increased later if needed

---

## Your Tuned Values

**Date Tuned:** _____________

**Surface:** ☐ Foam Tiles  ☐ Carpet  ☐ Competition Field

**Final Angular PID:**
```cpp
angularPID(
    ___,  // kP
    ___,  // kI
    ___,  // kD
    3, 1, 100, 3, 500, 0
)
```

**Final Lateral PID:**
```cpp
lateralPID(
    ___,  // kP
    ___,  // kI
    ___,  // kD
    3, 1, 100, 3, 500,
    ___   // Slew rate
)
```

**Odometry Configuration:**
```cpp
verticalTracking(&leftRotation, _______, _____);   // Diameter, Offset
horizontalTracking(&rearRotation, _______, _____); // Diameter, Offset
```

**Test Results:**
- Square Test Error: _____ inches
- Turn Test Error: _____ degrees
- Straight Test Error: _____ inches

**Notes:**
```
[Add any observations, issues, or specific tuning notes here]
```

---

**Good luck tuning! This process takes patience but results in reliable autonomous performance.**
