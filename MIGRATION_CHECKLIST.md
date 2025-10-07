# Season Migration Checklist
**Print this and check off items as you complete them!**

---

## Pre-Migration (Before touching code)

- [ ] **Back up current codebase**
  ```bash
  git add .
  git commit -m "[Pre-Migration] Backup before new season"
  git push
  ```

- [ ] **Create new branch for new season**
  ```bash
  git checkout -b season-2025-GAMENAME
  ```

- [ ] **Document robot wiring** (take photos of brain/ADI ports)

---

## Phase 1: Port Configuration (15 min)

### File: `include/globals.hpp`

- [ ] Update drivetrain motor ports (if changed)
- [ ] Remove old game mechanism port definitions
- [ ] Add new game mechanism port definitions
- [ ] Update sensor port definitions
- [ ] Update pneumatic ADI port definitions
- [ ] Remove any unused #define statements

**Test:** `make` should compile without errors

---

## Phase 2: Rename Generic Components (10 min)

### File: `src/globals.cpp` (Section 7)

- [ ] Rename `lib::Pneumatic` variables to match new game
  - [ ] Update variable names
  - [ ] Update comments

### File: `include/globals.hpp`

- [ ] Update `extern lib::Pneumatic` declarations
- [ ] Remove old extern declarations

**Test:** `make` should compile without errors

---

## Phase 3: Update/Create Subsystems (30 min)

### Option A: Simple Motor Control
- [ ] Use `lib::MotorSubsystem` directly in globals.cpp
- [ ] No custom class needed

### Option B: Complex Mechanisms
- [ ] Copy template: `templates/motor_subsystem_template.cpp`
- [ ] Create header: `include/robot/mechanism_name.hpp`
- [ ] Create implementation: `src/subsystems/mechanism_name.cpp`
- [ ] Update globals.cpp to instantiate new subsystem
- [ ] Update globals.hpp with extern declaration

### Clean Up Old Code:
- [ ] Delete old game subsystems from `include/robot/`
- [ ] Delete old game implementations from `src/subsystems/`
- [ ] Remove old subsystem externs from globals.hpp
- [ ] Remove old subsystem instantiation from globals.cpp

**Test:** `make` should compile without errors

---

## Phase 4: Autonomous Routines (20 min)

### File: `src/subsystems/auton.cpp`

- [ ] Delete all old autonomous routines
- [ ] Create new autonomous routine 1
- [ ] Create new autonomous routine 2
- [ ] Create new autonomous routine 3 (if needed)
- [ ] Update auton.hpp with new function declarations

### File: `src/main.cpp` → `autonomous()`

- [ ] Update to call new autonomous routine
- [ ] Comment out unused routines

**Test:** Run autonomous on practice field

---

## Phase 5: Driver Control (15 min)

### File: `src/main.cpp` → `opcontrol()`

- [ ] Update pneumatic button mappings
  ```cpp
  newMechanism.run(DIGITAL_BUTTON);
  ```
- [ ] Update motor subsystem controls
- [ ] Test all buttons on controller
- [ ] Update control comments

### File: `src/main.cpp` → `initialize()`

- [ ] Update game-specific initialization (if any)
- [ ] Set default states for mechanisms

**Test:** Test all controls on actual robot

---

## Phase 6: Verification & Testing (20 min)

### Compilation
- [ ] `make` compiles with no errors
- [ ] `make` compiles with no warnings (if possible)

### Hardware Test
- [ ] All motors spin in correct direction
  - [ ] Drivetrain left
  - [ ] Drivetrain right
  - [ ] Mechanism 1
  - [ ] Mechanism 2
  - [ ] Mechanism 3
- [ ] All pneumatics extend/retract correctly
  - [ ] Pneumatic 1
  - [ ] Pneumatic 2
- [ ] All sensors read correct values
  - [ ] IMU calibrates
  - [ ] Rotation sensors (if used)
  - [ ] Other sensors

### Functionality Test
- [ ] Autonomous runs without errors
- [ ] Driver control is responsive
- [ ] No unexpected behavior
- [ ] Emergency stop works

---

## Phase 7: Documentation (10 min)

### Update CONTEXT.md
- [ ] Change game name throughout document
- [ ] Update "Status" line at top
- [ ] Add notes about new season in progress tracker

### Update Driver Controls Documentation
- [ ] Document button layout
- [ ] Create driver cheat sheet (optional)

### Git Commit
- [ ] Stage all changes: `git add .`
- [ ] Commit: `git commit -m "[Migration] New season setup complete"`
- [ ] Push: `git push -u origin season-2025-GAMENAME`

---

## Phase 8: Competition Prep (Ongoing)

- [ ] Tune PID values (lateralPID, angularPID in globals.cpp)
- [ ] Test autonomous routines on competition field
- [ ] Practice driver control
- [ ] Document any special procedures
- [ ] Create pre-match checklist

---

## 🚨 CRITICAL: DO NOT MODIFY

**Never change these files (they're universal!):**
- ❌ `lib/Pneumatic.hpp`
- ❌ `lib/Pneumatic.cpp`
- ❌ `lib/MotorSubsystem.hpp`
- ❌ `lib/MotorSubsystem.cpp`
- ❌ `robot_config.hpp`
- ❌ `robot_config.cpp`
- ❌ LemLib library files
- ❌ PROS library files

---

## ✅ Success Criteria

Your migration is complete when:
- ✅ Code compiles without errors
- ✅ All mechanisms work correctly
- ✅ Autonomous runs successfully
- ✅ Driver controls are intuitive
- ✅ Code is committed to git
- ✅ Robot is ready for competition

**Expected Time: 1-2 hours** (vs. days with old codebase!)

---

## 🆘 Troubleshooting

### Problem: Code won't compile
- Check for typos in variable names
- Verify all port numbers are defined
- Make sure externs match actual variable names

### Problem: Motors spin backwards
- Swap motor polarity in port definition:
  ```cpp
  pros::Motor motor(-PORT, ...);  // Negative reverses
  ```

### Problem: Pneumatics work backwards
- Use opposite methods (extend ↔ retract)
- OR swap tubing on solenoid

### Problem: Robot doesn't move in autonomous
- Verify chassis.calibrate() is called
- Check autonomous routine is uncommented in main.cpp
- Test motor directions manually first

---

**Good luck with the new season! 🏆**

---

*For detailed examples, see: `templates/NEW_SEASON_SETUP.md`*
*For code templates, see: `templates/` directory*
