# Gain Sweep Setup Guide

## What the Robot Does

**35 PD gain tests @ 24 inches each, alternating forward/backward**

### Test Pattern:
```
Test 1:  Forward 24"   (kP4_kD0)
Test 2:  Backward 24"  (kP4_kD1)
Test 3:  Forward 24"   (kP4_kD2)
Test 4:  Backward 24"  (kP6_kD0)
... (continues alternating for 35 tests)
```

**Duration:** ~70 seconds total
- 1.5 seconds per test
- 0.5 second pause between tests
- Fits perfectly in 60s programming skills!

---

## Physical Setup

### Space Requirements:
- **Forward/Backward:** 30 inches clear (24" target + 6" safety margin)
- **Sides:** 12 inches clear each side (drift margin)

### Field Setup (12 ft long):
```
        ← 30" clear →
[WALL] ←→ [ROBOT] ←→ [WALL]
          ↕ 24"
```

### Recommended Starting Position:
1. Place robot **36 inches from wall** (gives 30" + 6" buffer)
2. Point forward direction parallel to wall
3. Ensure 12" clearance on left/right sides

---

## Running the Test

### 1. Build & Upload
```bash
make
# Upload via PROS GUI or connect USB and run programming skills
```

### 2. Run Programming Skills
- Connect field control or competition switch
- Select "Programming Skills" mode
- Robot will automatically start gain sweep

### 3. Monitor Progress
**Brain LCD shows:**
```
Line 0: GAIN SWEEP STARTED
Line 1: Distance: 24.0 in
Line 2: Tests: 35
Line 3: Test 15/35: kP10_kD3 FWD
Line 4: kP=10.0 kD=3.0
```

### 4. After Completion
**Brain LCD shows:**
```
GAIN SWEEP COMPLETE!
Tests: 35
Saved: /usd/gain_sweep.csv

Run tools/rank_gains.py
```

---

## Analysis

### 1. Pull SD Card
Remove SD card from V5 brain

### 2. Run Analysis Script
```bash
python tools/rank_gains.py
```

### 3. View Results
Script generates:
- **Console output:** Top 10 ranked gain sets
- **File:** `gain_ranking.txt` (detailed report)
- **Plot:** `top_3_gains.png` (visual comparison)

### 4. Apply Winner
Update `src/globals.cpp`:
```cpp
lemlib::ControllerSettings lateralPID(
    X.X,  // kP - use recommended value
    0,    // kI
    Y.Y,  // kD - use recommended value
    // ... rest unchanged
);
```

---

## Gain Test Grid (35 combinations)

### Low kP (4-6):
- kP=4: kD=[0,1,2]
- kP=6: kD=[0,1,2,3]

### Mid-low kP (8):
- kP=8: kD=[0,1,2,3,4]

### Sweet spot kP (10):
- kP=10: kD=[0,1,2,3,4,5,6]  ← Most variations!

### Mid-high kP (12):
- kP=12: kD=[0,2,3,4,5,6]

### High kP (14):
- kP=14: kD=[0,2,3,4,5]

### Very high kP (16):
- kP=16: kD=[2,3,4,5]

---

## Expected Behavior

### What You'll See:
- Some tests will **overshoot** (go past 24", then reverse back) → high kP, low kD
- Some tests will **undershoot** (stop short of 24") → low kP
- Some tests will be **smooth** (reach 24" cleanly) → optimal gains!

### Ranking Metrics:
1. **Settling time** - how fast it reaches target and stays there
2. **Overshoot** - how far past target (lower = better)
3. **Steady-state error** - final position accuracy
4. **Velocity reversals** - motor sign changes (0 = smooth)
5. **Smoothness** - motor power variance (lower = better)

---

## Troubleshooting

### Robot hits wall:
- Move starting position further from wall (48" minimum)
- Robot should oscillate within ±6" of target

### Not all tests complete:
- Check battery voltage (low battery = slower tests)
- Some high kP tests may oscillate longer

### SD card not saving:
- Verify SD card is inserted
- Check SD card has free space
- Brain LCD will show "NO SD CARD!" if missing

---

## What Happens Next

1. ✅ Run programming skills (robot does 35 tests)
2. ✅ Pull SD card
3. ✅ Run `python tools/rank_gains.py`
4. ✅ Get recommended kP, kD values
5. ✅ Update `globals.cpp`
6. ✅ Rebuild, test, done!

**Total time:** 5 minutes (vs. hours of manual tuning!)
