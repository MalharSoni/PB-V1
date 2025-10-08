# Quick Upload & Test Guide - LVGL Logo Integration

## Step 1: Upload to Robot

### Method 1: PROS CLI (Recommended)
```bash
# From HalloweenQualCode directory
pros upload

# If port selection needed
pros upload --port /dev/tty.usbmodem[YOUR_PORT]
```

### Method 2: PROS GUI
1. Open PROS for VS Code
2. Click "Upload" button in status bar
3. Select correct V5 Brain port

---

## Step 2: Test Sequence

### Test 1: Boot (No Crash!)
**Expected**:
- Robot powers on
- Brain screen shows green text:
  ```
  =============================

     /// CAUTION TAPE ///
     /// ROBOTICS CLUB ///

        TEAM 839Y

  =============================
  ```
- No "Data abort exception" error
- Robot stays running

**If it crashes**: Something went wrong. Check that main.cpp has NO brainUI calls in initialize()

---

### Test 2: Driver Control Start
**Action**: Press "Driver Control" on competition controller or brain

**Expected**:
1. **Splash Screen** (2 seconds)
   - Black background
   - CTRC logo in center (yellow diagonal stripes)
   - "Initializing..." text below logo
   - "Team 839Y" at bottom

2. **Operation Screen** (after 2 seconds)
   - Black background
   - Gold title: "ROBOT STATUS"
   - Position: X/Y coordinates
   - Heading: degrees
   - Battery: percentage
   - Motor temps: left/right

**If no UI appears**: Check PROS terminal for errors. LVGL task may not have started.

---

### Test 3: Telemetry Updates
**Action**: Drive robot around

**Expected**:
- X/Y position values change on screen
- Heading updates when turning
- Battery percentage updates
- Temperature values change

**If values don't update**: Check that `brainUI.updateTelemetry()` is being called in main loop

---

### Test 4: Logo Quality
**Action**: Look closely at logo on splash screen

**Expected**:
- Yellow diagonal stripes visible
- Clear edges (not pixelated)
- Centered on screen
- Proper colors (yellow/white, not corrupted)

**If logo looks wrong**:
- Check `include/logo.h` file size (should be ~82 KB)
- Verify conversion script ran correctly
- May need to regenerate logo data

---

## Quick Debug Commands

### Check PROS Terminal Output
```bash
# Open PROS terminal after upload
prosv5 terminal

# Look for:
# - "LVGL Task" starting
# - No crash messages
# - UI initialization logs
```

### Rebuild from Scratch
```bash
make clean
make
pros upload
```

### Verify Files Changed
```bash
# Check logo.h exists and has data
wc -l include/logo.h
# Should output: 1709 include/logo.h

# Check main.cpp has opcontrol() changes
grep -A 5 "brainUI.init()" src/main.cpp
# Should show it's in opcontrol(), not initialize()
```

---

## Success Criteria

✅ **PASS**: All of these must work
- [x] Robot boots without crash
- [x] LVGL splash screen appears in driver control
- [x] CTRC logo displays correctly
- [x] Telemetry values update during movement
- [x] No error messages in PROS terminal

---

## Common Issues & Fixes

### Issue: "Data abort exception"
**Cause**: LVGL initializing too early
**Fix**: Ensure `brainUI.init()` is in `opcontrol()`, NOT `initialize()`

### Issue: Black screen (no UI)
**Cause**: LVGL task handler not running
**Fix**: Check `brain_ui.cpp` has `lvgl_task_handler()` function and it's started in `init()`

### Issue: Logo doesn't appear
**Cause**: Image data not loaded or incorrect
**Fix**: Verify `include/logo.h` is 1,709 lines and contains `logo_ctrc_100x100_map[]`

### Issue: Telemetry stuck at 0
**Cause**: Chassis not initializing or telemetry not updating
**Fix**: Check `robot_config::initialize()` runs and `brainUI.updateTelemetry()` is called

---

## If Everything Works

🎉 **Congratulations!** Your LVGL UI with CTRC logo is working!

**Next Steps**:
1. Show your team the awesome logo on the brain screen
2. Practice with telemetry display during scrimmages
3. (Optional) Enable autonomous selector for competitions
4. Take photos for documentation/social media

---

## Upload Command Reference

```bash
# Standard upload
pros upload

# Upload with specific slot
pros upload --slot 1

# Upload and open terminal
pros upload && prosv5 terminal

# Quick rebuild and upload
make clean && make && pros upload
```

---

**Last Updated**: 2025-10-07
**Build Status**: ✅ Compiled successfully
**Ready to Upload**: YES
