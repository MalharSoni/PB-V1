# New Season Setup Guide

**Time Estimate:** 30-60 minutes
**Goal:** Adapt this codebase for a new VEX game season

---

## 📋 Quick Start Checklist

### Step 1: Update Port Definitions (5 minutes)
**File:** `include/globals.hpp`

```cpp
// Update these to match YOUR robot's wiring:

// Drivetrain (usually stays the same)
#define LEFT_MOTOR_FRONT 1
#define LEFT_MOTOR_MID 2
// ... etc

// YOUR game-specific mechanisms:
#define NEW_MECHANISM_PORT 10      // Rename these!
#define ANOTHER_MECHANISM_PORT 11
```

**Action Items:**
- [ ] Map all motor ports
- [ ] Map all sensor ports
- [ ] Map all pneumatic ADI ports
- [ ] Remove unused port definitions from last season

---

### Step 2: Rename Generic Components (10 minutes)
**File:** `src/globals.cpp` (Section 7: Generic Components)

```cpp
// OLD (High Stakes):
lib::Pneumatic clamp(CLAMP_PORT);
lib::Pneumatic doinker(DOINKER_PORT);

// NEW (YOUR game):
lib::Pneumatic wings(WINGS_PORT);          // Rename!
lib::Pneumatic blocker(BLOCKER_PORT);      // Rename!
```

**File:** `include/globals.hpp`

```cpp
// Update the extern declarations:
extern lib::Pneumatic wings;
extern lib::Pneumatic blocker;
```

**Action Items:**
- [ ] Rename all `lib::Pneumatic` variables to match your game
- [ ] Update both globals.cpp and globals.hpp
- [ ] Keep the same `lib::Pneumatic` class (don't change library code!)

---

### Step 3: Create Game-Specific Subsystems (20-30 minutes)

#### Option A: Simple Motor Control
If you just need basic motor control, **don't create a custom class!**

**File:** `src/globals.cpp`
```cpp
// Just use lib::MotorSubsystem directly:
pros::Motor shooterMotor(SHOOTER_PORT, pros::E_MOTOR_GEARSET_06);
lib::MotorSubsystem shooter({shooterMotor});
```

**File:** `src/main.cpp`
```cpp
// Control it directly:
if (master.get_digital(DIGITAL_R1)) {
    shooter.move(12000);  // Full speed
} else {
    shooter.stop();
}
```

#### Option B: Complex Mechanism (needs custom logic)
If you need sensors, algorithms, or special behavior:

1. **Copy the template:**
   ```bash
   cp templates/motor_subsystem_template.cpp include/robot/your_mechanism.hpp
   ```

2. **Rename and customize:**
   - Replace `MyMechanism` with your mechanism name
   - Add game-specific methods
   - Add sensors/logic as needed

3. **Implement in .cpp:**
   ```bash
   # Create implementation file
   touch src/subsystems/your_mechanism.cpp
   ```

4. **Update globals:**
   ```cpp
   // In globals.cpp
   subsystems::YourMechanism mechanism(...);
   ```

**Action Items:**
- [ ] Decide: simple motor control OR custom subsystem?
- [ ] Create custom subsystems only if needed
- [ ] Test each mechanism individually

---

### Step 4: Update Autonomous Routines (15-20 minutes)
**File:** `src/subsystems/auton.cpp`

```cpp
// Delete all old game autonomous routines
// Create new ones for YOUR game:

void Auton::newGameAuton1() {
    // YOUR autonomous code here

    // Use generic components (automatically renamed):
    wings.extend();
    chassis.moveToPose(24, 24, 0, 2000);
    wings.retract();
}
```

**File:** `src/main.cpp`
```cpp
void autonomous() {
    // Call your new autonomous:
    auton.newGameAuton1();
}
```

**Action Items:**
- [ ] Delete old autonomous routines
- [ ] Create 2-3 new autonomous routines
- [ ] Test on field

---

### Step 5: Update Driver Control (10 minutes)
**File:** `src/main.cpp` → `opcontrol()`

```cpp
void opcontrol() {
    while (true) {
        // Drivetrain (usually same)
        movement.arcade_drive(master.get_analog(ANALOG_LEFT_Y),
                             master.get_analog(ANALOG_RIGHT_X));

        // YOUR game controls:
        wings.run(DIGITAL_Y);          // Y button: wings
        blocker.run(DIGITAL_X);        // X button: blocker

        // Custom subsystem:
        if (master.get_digital(DIGITAL_R1)) {
            myMechanism.doSomething();
        }

        pros::delay(10);
    }
}
```

**Action Items:**
- [ ] Map all buttons to mechanisms
- [ ] Test all controls
- [ ] Update button layout documentation

---

### Step 6: Clean Up Old Code (5 minutes)

**Delete or archive:**
- [ ] Old game-specific subsystems (Intake/Arm if not needed)
- [ ] Old autonomous routines
- [ ] Unused sensor code

**Files to clean:**
- `src/subsystems/` - Remove old game files
- `include/robot/` - Remove old game headers
- `src/globals.cpp` - Remove old subsystem instantiation

---

## 🔧 What NOT to Change

**NEVER modify these (they're universal!):**
- ✅ `lib/Pneumatic.hpp` and `lib/Pneumatic.cpp`
- ✅ `lib/MotorSubsystem.hpp` and `lib/MotorSubsystem.cpp`
- ✅ `robot_config.hpp` and `robot_config.cpp`
- ✅ LemLib configuration (unless tuning PID)
- ✅ Build system (Makefile, project.pros)

**Only update these:**
- ✅ Port definitions (`globals.hpp`)
- ✅ Hardware instantiation (`globals.cpp`)
- ✅ Autonomous routines (`auton.cpp`)
- ✅ Driver control (`main.cpp`)
- ✅ Game-specific subsystems (`robot/` directory)

---

## 🎯 Expected Reuse Rate

With this framework:
- **80% of code is reusable** (lib/, robot_config, globals structure)
- **20% is game-specific** (subsystems, autonomous, controls)

**You should only need to write:**
1. Port definitions
2. Variable renaming
3. Game-specific subsystem logic (if complex)
4. Autonomous routines
5. Driver controls

---

## 🚨 Common Mistakes

### ❌ DON'T: Create new Pneumatic classes
```cpp
// BAD - Don't do this!
class Wings {
    pros::ADIDigitalOut piston;
    // ... reimplementing lib::Pneumatic
};
```

### ✅ DO: Just rename the variable
```cpp
// GOOD - Use existing lib::Pneumatic
lib::Pneumatic wings(WINGS_PORT);
```

---

### ❌ DON'T: Copy-paste motor control code
```cpp
// BAD - Duplicate code!
class Flywheel {
    void spin() {
        motors.move_voltage(12000);
        motors.get_positions();
        // ... etc
    }
};
```

### ✅ DO: Extend lib::MotorSubsystem
```cpp
// GOOD - Reuse base class
class Flywheel : public lib::MotorSubsystem {
    void spin() {
        move(12000);  // Inherited method!
    }
};
```

---

### ❌ DON'T: Modify library files
```cpp
// BAD - Don't change lib/Pneumatic.hpp!
namespace lib {
    class Pneumatic {
        void myGameSpecificMethod();  // NO!
    };
}
```

### ✅ DO: Create game-specific extensions if needed
```cpp
// GOOD - Extend, don't modify
namespace subsystems {
    class SpecialWings : public lib::Pneumatic {
        void myGameSpecificMethod();  // OK!
    };
}
```

---

## 🧪 Testing Checklist

After setup, verify:
- [ ] Robot compiles without errors
- [ ] All motors move in correct direction
- [ ] All pneumatics extend/retract correctly
- [ ] Sensors read correct values
- [ ] Autonomous runs without crashing
- [ ] Driver controls feel responsive
- [ ] No port conflicts

---

## 📝 Final Steps

1. **Update CONTEXT.md:**
   - Change game name from "High Stakes" to your game
   - Update week progress tracker
   - Document any new patterns

2. **Commit to git:**
   ```bash
   git add .
   git commit -m "[New Season] Setup for [GAME_NAME]"
   git push
   ```

3. **Test on actual robot!**

---

## 💡 Need Help?

**Check these resources:**
- `templates/pneumatic_template.cpp` - Pneumatic examples
- `templates/motor_subsystem_template.cpp` - Motor subsystem examples
- `CONTEXT.md` - Framework documentation
- `CLAUDE.md` - Development guide

**Remember:** Keep it simple! Don't create unnecessary complexity.

---

**Estimated Total Time:** 30-60 minutes
**Code Reuse:** 80%+
**Success Rate:** If you follow this guide, you'll have a working robot in under an hour! 🎉
