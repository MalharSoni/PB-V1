# CONTEXT.md - Universal VEX Framework Refactoring

**Project:** Team 839Y VEX Robotics Codebase Transformation
**Goal:** Convert High Stakes-specific code into a universal, game-agnostic framework
**Status:** Week 1 Complete, Week 2 In Progress
**Last Updated:** 2025-10-07

---

## 🎯 Project Vision

Transform a single-season codebase into a **professional, reusable framework** that:
- Works for ANY VEX game (not just High Stakes)
- Reduces new season setup time from days to ~1 hour
- Enables 80%+ code reuse across seasons
- Provides clear, student-friendly templates
- Maintains professional code quality standards

---

## 📊 Progress Tracker

### ✅ Week 1: Universal Pneumatic Component (COMPLETED)
- [x] Created `lib::Pneumatic` generic class
- [x] Replaced Clamp subsystem with Pneumatic instance
- [x] Replaced Doinker subsystem with Pneumatic instance
- [x] Updated all references in main.cpp and auton.cpp
- [x] Build tested and committed to git

### 🔄 Week 2: Motor Subsystem Base (IN PROGRESS)
- [ ] Create `lib::MotorSubsystem` base class
- [ ] Refactor Intake to extend MotorSubsystem
- [ ] Refactor Arm to extend MotorSubsystem
- [ ] Test and commit changes

### ⏳ Week 3: Configuration System (PENDING)
- [ ] Create `robot_config.cpp` centralized configuration
- [ ] Move all globals to config system
- [ ] Update main.cpp to use config

### ⏳ Week 4: Templates & Documentation (PENDING)
- [ ] Create templates/ directory with examples
- [ ] Write MIGRATION_CHECKLIST.md
- [ ] Update CLAUDE.md with framework documentation

---

## 🏗️ Architecture Decisions

### Design Philosophy: "Simple > Complex"

**Core Principle:** Favor **simple, copyable components** over complex inheritance hierarchies.

**Why:** High school students need to:
- Understand the code quickly
- Copy-paste for new mechanisms
- Modify without breaking the framework

### Component Types

1. **Generic Library Components** (`lib/` namespace)
   - Reusable across ALL games
   - Zero game-specific logic
   - Well-documented with examples
   - Examples: `lib::Pneumatic`, `lib::MotorSubsystem`

2. **Game-Specific Subsystems** (`robot/` namespace)
   - Extend lib components when needed
   - Contain game-specific behavior
   - Can be deleted/replaced each season
   - Examples: `Intake`, `Arm` (High Stakes specific)

3. **Configuration Files**
   - `globals.hpp` - Port definitions only
   - `robot_config.cpp` - All robot instantiation
   - `game_config.hpp` - Game constants (future)

---

## 📐 Code Quality Standards

### Naming Conventions

**Generic Components (lib/):**
```cpp
// Use generic, game-agnostic names
lib::Pneumatic        // NOT lib::Clamp or lib::Doinker
lib::MotorSubsystem   // NOT lib::Intake or lib::Arm
lib::SensorWrapper    // NOT lib::ColorSensor
```

**Student Variables:**
```cpp
// Students rename based on their game
lib::Pneumatic clamp(PORT);      // High Stakes
lib::Pneumatic wings(PORT);      // Future game
lib::Pneumatic blocker(PORT);    // Future game
```

**Method Naming:**
```cpp
// Provide multiple aliases for readability
void extend();     // Primary name
void activate();   // Alias
void open();       // Alias
void deploy();     // Alias
```

### Documentation Standards

**Every public method MUST have:**
1. Brief description
2. Parameter documentation
3. Usage example (when non-obvious)

**Example:**
```cpp
/**
 * @brief Extend the pneumatic (set to true)
 * Alias: activate(), open(), deploy()
 *
 * Usage in autonomous:
 *   clamp.extend();  // Grab mobile goal
 */
void extend();
```

### File Organization

```
include/
  lib/              # Generic, reusable components
    Pneumatic.hpp
    MotorSubsystem.hpp
  robot/            # Game-specific subsystems
    intake.hpp      # Extends lib components
    arm.hpp
  globals.hpp       # Port definitions + externs

src/
  lib/              # Generic implementations
    Pneumatic.cpp
    MotorSubsystem.cpp
  robot/            # Game-specific implementations
    intake.cpp
    arm.cpp
  globals.cpp       # Hardware instantiation
  robot_config.cpp  # Centralized config (future)
  main.cpp          # Entry points

templates/          # Student copy-paste examples
  pneumatic_example.cpp
  motor_example.cpp
  MIGRATION_CHECKLIST.md
```

---

## 🔧 Week 1 Implementation Details

### lib::Pneumatic Design

**Problem Solved:**
- Clamp and Doinker were identical code (116 lines total)
- Game-specific names made reuse difficult

**Solution:**
- Single generic class with multiple method aliases
- Students instantiate with game-specific variable names

**Key Features:**
```cpp
lib::Pneumatic mechanism(port);
mechanism.toggle();      // Toggle state
mechanism.extend();      // Set to true
mechanism.retract();     // Set to false
mechanism.run(BUTTON);   // Controller integration
```

**Migration Pattern:**
```cpp
// OLD (High Stakes only)
subsystems::Clamp clamp(PORT);
clamp.clamp_stake();

// NEW (Universal)
lib::Pneumatic clamp(PORT);
clamp.extend();
```

**Files Changed:**
- `include/lib/Pneumatic.hpp` (new)
- `src/lib/Pneumatic.cpp` (new)
- `include/globals.hpp` (updated externs)
- `src/globals.cpp` (updated instantiation)
- `src/subsystems/auton.cpp` (updated method calls)

**Build Impact:**
- ✅ Compiles successfully
- ✅ No functionality changes
- ✅ All autonomous routines updated

---

## 🎓 Week 2 Design Plan

### lib::MotorSubsystem Base Class

**Problem to Solve:**
- Intake and Arm both use `pros::MotorGroup`
- Common patterns: move(), stop(), getPosition(), etc.
- Lots of duplicated code

**Solution:**
- Extract common motor operations to base class
- Intake/Arm extend and add game-specific behavior

**Common Operations Identified:**
```cpp
// From Intake:
intake_motors.move_voltage(voltage);
intake_motors.move_relative(position, speed);
intake_motors.get_positions();
intake_motors.set_zero_position();

// From Arm:
armMotor.move_absolute(position, speed);
armMotor.move_voltage(voltage);
armMotor.move_relative(position, speed);
```

**Base Class Interface (Planned):**
```cpp
namespace lib {
    class MotorSubsystem {
    public:
        MotorSubsystem(std::vector<pros::Motor> motors);

        // Basic motor control
        void move(int voltage);
        void moveAbsolute(float position, float speed = 100);
        void moveRelative(float position, float speed = 100);
        void stop();

        // State queries
        float getPosition();
        float getVelocity();
        float getTemperature();

        // Configuration
        void setBrakeMode(pros::motor_brake_mode_e mode);
        void setZeroPosition(float position = 0);

    protected:
        pros::MotorGroup motors;
    };
}
```

**Intake Extension (Planned):**
```cpp
class Intake : public lib::MotorSubsystem {
public:
    Intake(std::vector<pros::Motor> motors, int color_sensor_port, int piston_port);

    // Game-specific: High Stakes color sorting
    void startColourSort(DONUT_COLOR alliance);
    void stopColourSort();

    // Inherits from MotorSubsystem:
    //   move(), stop(), getPosition(), etc.

private:
    pros::Optical color_sensor;
    lib::Pneumatic intake_piston;
    DONUT_COLOR target_color;
};
```

---

## ⚠️ Important Constraints

### DO NOT Modify:
- LemLib library files (`include/lemlib/`, `firmware/LemLib.a`)
- PROS library files (`include/pros/`, `firmware/libpros.a`)
- OkapiLib files (`include/okapi/`, `firmware/okapilib.a`)

### DO Modify Carefully:
- `globals.hpp` - Only add/reorganize, don't break existing code
- `main.cpp` - Keep entry points compatible
- Existing subsystems - Ensure backward compatibility during refactor

### Testing Requirements:
- MUST compile successfully after each major change
- MUST verify no functionality regressions
- MUST test at least one autonomous routine

---

## 🚨 Common Pitfalls to Avoid

### ❌ Don't: Create complex inheritance hierarchies
```cpp
// BAD - Too complex for students
BaseSubsystem → MotorizedSubsystem → IntakeBase → ColorSortingIntake
```

### ✅ Do: Keep it simple
```cpp
// GOOD - One level of inheritance
lib::MotorSubsystem → Intake
```

### ❌ Don't: Use game-specific names in lib/
```cpp
// BAD - lib components should be generic
namespace lib {
    class MobileGoalClamp { };  // Too specific!
}
```

### ✅ Do: Use generic names in lib/
```cpp
// GOOD - Generic and reusable
namespace lib {
    class Pneumatic { };
}
```

### ❌ Don't: Hardcode game constants in lib/
```cpp
// BAD - High Stakes specific
lib::Pneumatic::extend() {
    // Extend to grab mobile goal  // NO!
}
```

### ✅ Do: Keep lib/ game-agnostic
```cpp
// GOOD - Generic description
lib::Pneumatic::extend() {
    // Extend pneumatic piston  // YES!
}
```

---

## 📝 Code Review Checklist

Before committing any change:

- [ ] Code compiles without errors
- [ ] Code compiles without warnings (if reasonable)
- [ ] All public methods have documentation
- [ ] Generic components use game-agnostic naming
- [ ] No hardcoded game-specific values in lib/
- [ ] At least one autonomous routine tested
- [ ] Git commit message follows format
- [ ] CONTEXT.md updated if architecture changed

### Git Commit Message Format

```
[Week X] Brief description

Detailed explanation of what changed and why.

Changes:
- Created/Updated file X
- Refactored Y to use Z
- Fixed issue with W

Build tested: ✓
Functionality: ✓ Description

🤖 Generated with Claude Code (https://claude.com/claude-code)

Co-Authored-By: Claude <noreply@anthropic.com>
```

---

## 🎯 Success Metrics

### Week 1 Metrics (Achieved):
- ✅ Code reduction: 116 lines → 1 reusable class
- ✅ Build time: No regression
- ✅ Lines of documentation: 93 lines (comprehensive)
- ✅ Breaking changes: 0 (backward compatible method names)

### Week 2 Target Metrics:
- Reduce motor subsystem duplication by 50%+
- Intake/Arm classes become 30% smaller
- All motor operations go through base class
- Zero functionality regressions

### Overall Project Targets:
- New season setup time: < 1 hour (from days)
- Code reuse: 80%+ across seasons
- Student onboarding: < 30 minutes to understand framework
- Template usage: Students copy-paste without modification

---

## 🔗 Related Documentation

- **CLAUDE.md** - Full development guide for AI assistants
- **README.md** - User-facing project documentation (if exists)
- **templates/MIGRATION_CHECKLIST.md** - Season migration guide (pending)
- **docs/ARCHITECTURE.md** - Framework architecture (future)

---

## 💡 Future Enhancements (Post-Week 4)

### Phase 5: Advanced Features
- Unit testing framework
- Telemetry & data logging
- Field coordinate manager
- Auto-generated documentation

### Phase 6: Developer Tools
- Web-based configuration generator
- Visual port mapper
- Autonomous path visualizer
- Match replay system

---

## 🤝 Contribution Guidelines

### For Students Adding New Subsystems:

1. **Check if lib/ component exists**
   - Pneumatic? Use `lib::Pneumatic`
   - Motor-driven? Extend `lib::MotorSubsystem`

2. **Only create custom class if:**
   - Behavior is truly game-specific
   - Can't be achieved with lib/ components

3. **Name variables based on YOUR game:**
   ```cpp
   lib::Pneumatic wings(PORT);  // Your game
   lib::Pneumatic clamp(PORT);  // Not someone else's game
   ```

4. **Document your code:**
   - What does this mechanism do in YOUR game?
   - How should future programmers use it?

---

## 📞 Getting Help

### For Framework Questions:
- Read this CONTEXT.md file
- Check CLAUDE.md for detailed examples
- Look at templates/ for copy-paste examples

### For PROS/LemLib Questions:
- PROS Documentation: https://pros.cs.purdue.edu/
- LemLib Docs: https://lemlib.readthedocs.io/

### For VEX Game Rules:
- VEX Robotics: https://www.vexrobotics.com/

---

**Remember:** Keep it simple. Keep it reusable. Keep it documented.

**Last Refactored By:** Claude Code
**Team:** 839Y
**Season:** 2024-2025 (High Stakes) → Universal Framework
