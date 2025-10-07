# High Stakes Archived Subsystems

These files have been **replaced by the universal framework** and are no longer needed in active code.

## Archived Files

### Replaced by `lib::Pneumatic`:
- ❌ `clamp.hpp/.cpp` → Now use `lib::Pneumatic clamp(PORT);`
- ❌ `doinker.hpp/.cpp` → Now use `lib::Pneumatic doinker(PORT);`

### Old/Unused:
- ❌ `old_intake.cpp/.hpp` → Replaced by refactored intake

## Why Archived?

These subsystems were game-specific duplicates. The framework now provides:
- **`lib::Pneumatic`** - Universal pneumatic control (no need for custom clamp/doinker classes)
- **`lib::MotorSubsystem`** - Universal motor base class

## For New Seasons

**Don't use these files!** Instead:
1. Use `lib::Pneumatic` for pneumatics
2. Extend `lib::MotorSubsystem` for motors
3. See `templates/` for examples

These files are kept for reference only.
