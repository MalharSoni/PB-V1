# Tracking Wheel Diagnostic Guide

## Problem
Both X and Y coordinates are changing when spinning either tracking wheel individually. This means the wheels or ports are configured incorrectly.

## Solution - Step by Step Test

### Step 1: Upload and View Diagnostic
1. Upload code: `pros upload`
2. Look at brain screen - you should see:
   ```
   =============================

      /// CAUTION TAPE ///
      /// TEAM 839Y ///

   L:0 R:0                    ← Raw sensor values
   X:0.00 Y:0.00 H:0.0       ← Position
   =============================
   ```

### Step 2: Identify Physical Wheels

**Lift robot off ground so wheels can spin freely**

#### Test A: Left Sensor (Port 8)
1. Look at brain screen line 5: Note the `L:` value
2. **Manually spin ONLY the tracking wheel on PORT 8**
3. Watch line 5: The `L:` number should change
4. **Which direction does this wheel measure?**
   - If it's mounted on the SIDE (left/right): It's your **VERTICAL** wheel
   - If it's mounted at FRONT/REAR: It's your **HORIZONTAL** wheel

#### Test B: Rear Sensor (Port 7)
1. Look at brain screen line 5: Note the `R:` value
2. **Manually spin ONLY the tracking wheel on PORT 7**
3. Watch line 5: The `R:` number should change
4. **Which direction does this wheel measure?**
   - If it's mounted on the SIDE (left/right): It's your **VERTICAL** wheel
   - If it's mounted at FRONT/REAR: It's your **HORIZONTAL** wheel

### Step 3: Record Your Results

**Port 8 (LEFT_ROTATION):**
- ☐ Vertical wheel (measures forward/backward)
- ☐ Horizontal wheel (measures left/right strafe)

**Port 7 (REAR_ROTATION):**
- ☐ Vertical wheel (measures forward/backward)
- ☐ Horizontal wheel (measures left/right strafe)

---

## Fix Based on Results

### Case 1: Ports are Correct ✅
**If Port 8 = Vertical AND Port 7 = Horizontal:**
- Your ports are fine!
- Problem is likely the "reversed" flag
- Jump to "Fix Option C" below

### Case 2: Ports are Swapped ❌
**If Port 8 = Horizontal AND Port 7 = Vertical:**
- Ports are backwards!
- Jump to "Fix Option A" below

---

## Fix Options

### Fix Option A: Swap Port Definitions (Ports Swapped)

**Edit `include/globals.hpp` lines 60-61:**

**Change FROM:**
```cpp
#define LEFT_ROTATION 8     // Left tracking wheel (odometry)
#define REAR_ROTATION 7     // Rear tracking wheel (odometry)
```

**Change TO:**
```cpp
#define LEFT_ROTATION 7     // Left tracking wheel (odometry) - SWAPPED
#define REAR_ROTATION 8     // Rear tracking wheel (odometry) - SWAPPED
```

**What this does:** Swaps which port number is used for vertical vs horizontal tracking

---

### Fix Option B: Swap Sensor Objects (Less Common)

If you want to keep port numbers the same but swap sensor assignments:

**Edit `src/globals.cpp` lines 63-64:**

**Change FROM:**
```cpp
lemlib::TrackingWheel verticalTracking(&leftRotation, lemlib::Omniwheel::NEW_275, 0.0);
lemlib::TrackingWheel horizontalTracking(&rearRotation, lemlib::Omniwheel::NEW_275, -4.0);
```

**Change TO:**
```cpp
lemlib::TrackingWheel verticalTracking(&rearRotation, lemlib::Omniwheel::NEW_275, -4.0);
lemlib::TrackingWheel horizontalTracking(&leftRotation, lemlib::Omniwheel::NEW_275, 0.0);
```

---

### Fix Option C: Fix Reversed Flags

If ports are correct but readings are inverted:

**Edit `src/globals.cpp` lines 54-55:**

**Current:**
```cpp
pros::Rotation leftRotation(LEFT_ROTATION, true);     // Reversed
pros::Rotation rearRotation(REAR_ROTATION, true);     // Reversed
```

**Try these combinations:**

**Option C1 - Neither reversed:**
```cpp
pros::Rotation leftRotation(LEFT_ROTATION, false);
pros::Rotation rearRotation(REAR_ROTATION, false);
```

**Option C2 - Only left reversed:**
```cpp
pros::Rotation leftRotation(LEFT_ROTATION, true);
pros::Rotation rearRotation(REAR_ROTATION, false);
```

**Option C3 - Only rear reversed:**
```cpp
pros::Rotation leftRotation(LEFT_ROTATION, false);
pros::Rotation rearRotation(REAR_ROTATION, true);
```

---

## Validation Test

After applying fix, test again:

### Test 1: Vertical Wheel (Forward/Backward)
1. Spin your VERTICAL tracking wheel manually
2. Watch brain screen line 6: `X:___ Y:___ H:___`
3. **✅ ONLY Y should change**
4. **❌ If X changes: Configuration still wrong**

### Test 2: Horizontal Wheel (Left/Right)
1. Spin your HORIZONTAL tracking wheel manually
2. Watch brain screen line 6: `X:___ Y:___ H:___`
3. **✅ ONLY X should change**
4. **❌ If Y changes: Configuration still wrong**

### Test 3: Drive Test
1. Drive robot FORWARD in a straight line
2. Watch line 6: Y should increase, X should stay ~0
3. Drive robot in a CIRCLE
4. Both X and Y should change smoothly

**If all tests pass:** ✅ Configuration is correct!

---

## Most Likely Fix

**Based on typical VEX setups, try Fix Option A first (swap ports).**

Most teams wire:
- Port 7 or 8: Side-mounted vertical wheel
- Port 7 or 8: Rear-mounted horizontal wheel

The current config assumes port 8 is vertical and port 7 is horizontal. If yours are reversed physically, swapping the port definitions will fix it.

---

## Quick Fix Summary

1. ✅ Upload current code with diagnostics
2. ✅ Test which physical wheel is on which port
3. ✅ Apply appropriate fix (most likely Option A)
4. ✅ Rebuild: `make`
5. ✅ Upload: `pros upload`
6. ✅ Run validation tests
7. ✅ If still wrong, try Option C (reversed flags)

---

**After fixing, run the odometry square test to verify accuracy!**
```cpp
// In autonomous():
auton.odomSquareTest();
```
