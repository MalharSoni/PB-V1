# 839Y Push Back Robot - Testing Checklist

## ✅ Fixed Issues

### 1. Port Conflicts Resolved
**Problem:** All sensors were assigned to port 22, causing conflicts
**Solution:** Reassigned unique ports to all sensors

**New Port Assignments:**
```
MOTORS (V5 Smart Ports):
├─ Drivetrain: 11, 12, 13, 14, 15, 16 (6 motors)
└─ Intake:     18, 19, 20 (3 motors)

SENSORS (V5 Smart Ports):
├─ IMU (Inertial):        10
├─ Left Tracking Wheel:    1
├─ Rear Tracking Wheel:    2
├─ Right Distance Sensor:  3
└─ Left Distance Sensor:   4

PNEUMATICS (ADI Ports):
├─ Clamp:    C
├─ Doinker:  H
├─ Intake Piston: B (if used)
└─ Arm Piston:    D (if used)
```

### 2. Tracking Wheels Enabled
- Odometry now uses both tracking wheels + IMU for better accuracy
- Previously was IMU-only due to port conflicts

### 3. Push Back Intake Implemented
- 3-stage system with 5 operational modes
- 4-button driver control configured

### 4. Build Status
✅ **Build Successful** - All compilation errors fixed

---

## 🧪 Testing Checklist

### Phase 1: Hardware Verification (BEFORE powering on)

#### Port Verification
- [ ] **Port 1:** Left tracking wheel rotation sensor connected
- [ ] **Port 2:** Rear tracking wheel rotation sensor connected
- [ ] **Port 3:** Right distance sensor connected
- [ ] **Port 4:** Left distance sensor connected
- [ ] **Port 10:** IMU sensor connected
- [ ] **Ports 11-16:** Drivetrain motors connected (verify left/right sides)
- [ ] **Port 18:** Intake Stage 3 motor (scoring)
- [ ] **Port 19:** Intake Stage 2 motor (transfer/routing)
- [ ] **Port 20:** Intake Stage 1 motor (main intake)
- [ ] **ADI Port C:** Clamp pneumatic
- [ ] **ADI Port H:** Doinker pneumatic

#### Motor Direction Check
Upload code and verify motor directions match:
- [ ] Left drivetrain motors (15, 14, 11) - should be **reversed** (negative ports)
- [ ] Right drivetrain motors (16, 12, 13) - should be **normal** (positive ports)
- [ ] Intake motors - verify during intake testing

---

### Phase 2: Driver Control Testing

#### Drivetrain
- [ ] Left joystick Y-axis controls forward/backward
- [ ] Right joystick X-axis controls turning
- [ ] Robot drives straight when pushing forward
- [ ] Brake mode: Robot stops immediately when joystick released

#### Intake System (4-Button Control)
Test each button individually:

**R1 Button - Intake from Floor:**
- [ ] Stage 1 spins at full forward (pulls balls in)
- [ ] Stage 2 spins at half forward (transfers)
- [ ] Stage 3 is off
- [ ] Balls are successfully picked up from floor

**R2 Button - Score Level 1 / Outtake:**
- [ ] Stage 1 spins at full reverse (pushes balls out)
- [ ] Stage 2 spins at half reverse (assists)
- [ ] Stage 3 is off
- [ ] Balls successfully eject or score low

**L1 Button - Score Level 2:**
- [ ] Stage 1 is off
- [ ] Stage 2 spins at full forward (routing)
- [ ] Stage 3 spins at 3/4 forward (scoring)
- [ ] Balls successfully score at mid-level

**L2 Button - Score Level 3:**
- [ ] Stage 1 is off
- [ ] Stage 2 spins at full forward (routing)
- [ ] Stage 3 spins at full forward (high scoring)
- [ ] Balls successfully score at high-level

**No Button Pressed:**
- [ ] All intake motors stop immediately

#### Pneumatics
- [ ] **Y Button:** Clamp extends/retracts correctly
- [ ] **Right Button:** Doinker extends/retracts correctly

#### Wall Alignment (Optional Features)
- [ ] **A Button:** Aligns robot angle to wall (if distance sensors installed)
- [ ] **X Button:** Aligns robot distance to wall

---

### Phase 3: Odometry & Tracking

#### Brain LCD Display
Check that values update correctly:
- [ ] Row 1: X position updates when robot moves left/right
- [ ] Row 2: Y position updates when robot moves forward/back
- [ ] Row 3: Theta (heading) updates when robot turns

#### Controller Display
- [ ] X and Y coordinates display on controller screen

#### Tracking Wheel Verification
- [ ] Left tracking wheel (port 1) spins when robot moves forward
- [ ] Rear tracking wheel (port 2) spins when robot strafes (if applicable)
- [ ] IMU heading updates smoothly when turning

---

### Phase 4: Motor Speed Tuning

**⚠️ The intake motor speeds are currently set to default values and NEED TUNING!**

Current speeds (in `src/subsystems/intake_pushback.cpp`):

```cpp
// Mode 1: Intake
stage1_motor.move_voltage(12000);  // Full forward
stage2_motor.move_voltage(6000);   // Half forward
stage3_motor.move_voltage(0);      // Off

// Mode 2: Score Level 1 (Outtake)
stage1_motor.move_voltage(-12000); // Full reverse
stage2_motor.move_voltage(-6000);  // Half reverse
stage3_motor.move_voltage(0);      // Off

// Mode 3: Store
stage1_motor.move_voltage(0);      // Off
stage2_motor.move_voltage(3000);   // Slow hold
stage3_motor.move_voltage(0);      // Off

// Mode 4: Score Level 2
stage1_motor.move_voltage(0);      // Off
stage2_motor.move_voltage(12000);  // Full forward
stage3_motor.move_voltage(9000);   // 3/4 forward

// Mode 5: Score Level 3
stage1_motor.move_voltage(0);      // Off
stage2_motor.move_voltage(12000);  // Full forward
stage3_motor.move_voltage(12000);  // Full forward
```

**Tuning Process:**
1. Test each mode with current speeds
2. Adjust voltages (range: -12000 to +12000 millivolts)
3. Find optimal speeds for smooth ball transfer
4. Update values in `intake_pushback.cpp`

---

### Phase 5: Known Issues & Notes

#### IDE Diagnostics (Red Squiggles)
- **Status:** Ignore clang intellisense errors in IDE
- **Reason:** Build compiles successfully, these are just IDE configuration issues
- **Files affected:** globals.hpp shows false "not found" errors

#### Autonomous Routines
- **Status:** Old High Stakes autonomous routines are commented out
- **Action needed:** Write new autonomous routines for Push Back game
- **Location:** `src/subsystems/auton.cpp`
- **Available in main.cpp:** Lines 57-64 show autonomous options (currently empty)

#### Motor Gearsets
- **Note:** Code uses deprecated gearset naming (E_MOTOR_GEARSET_18, 06)
- **Effect:** None - code compiles and runs correctly
- **If issues occur:** Update to modern naming (E_MOTOR_GEAR_BLUE, GREEN, RED)

---

## 🔧 Common Issues & Fixes

### Issue: Motor spins backward
**Fix:** Add/remove negative sign in port definition in `include/globals.hpp`

### Issue: Intake doesn't pick up balls
**Fix:** Tune motor speeds in `src/subsystems/intake_pushback.cpp`

### Issue: Tracking wheels not reading
**Fix:**
1. Verify rotation sensors are plugged into ports 1 and 2
2. Check that sensors are properly mounted to tracking wheels
3. Verify reversed setting matches your hardware

### Issue: Distance sensors not working
**Fix:**
1. Verify sensors plugged into ports 3 and 4
2. Check sensor type matches code (currently using pros::Distance)

### Issue: Robot position drifts during autonomous
**Fix:**
1. Calibrate IMU during initialize() (already done automatically)
2. Tune tracking wheel offsets in `src/globals.cpp` lines 63-64
3. Adjust PID values in `src/globals.cpp` lines 121-144

---

## 📝 Next Steps After Testing

1. **Tune intake motor speeds** based on testing results
2. **Write autonomous routines** for Push Back game
3. **Calibrate odometry** if position tracking is inaccurate
4. **Test pneumatics** timing (clamp/doinker actuation speed)
5. **Practice driver control** to optimize button layout

---

## 🤖 Ready to Test!

**Build Status:** ✅ Successful
**All port conflicts:** ✅ Fixed
**Tracking wheels:** ✅ Enabled
**Push Back intake:** ✅ Implemented

Upload to robot and start testing! 🚀
