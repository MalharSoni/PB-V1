# AI-Optimized 15-Second Autonomous Route

**Generated:** 2025-10-08
**Strategy:** Safe Corner Sweep
**Estimated Time:** 14-15 seconds

---

## 🎯 Route Overview

This route is designed for maximum reliability and scoring while respecting all autonomous constraints:

✅ **Starts touching park barrier** - (-60, -60)
✅ **Never crosses autonomous line** - Stays X < 0
✅ **Collects 3 balls** - Conservative but reliable
✅ **Scores at zone** - Level 2 scoring
✅ **Ends at park barrier** - (-60, -54)

---

## 📍 Waypoint Breakdown

| # | Position | Heading | Speed | Time | Action |
|---|----------|---------|-------|------|--------|
| **Start** | (-60, -60) | 45° | - | 0s | Start intake |
| **1** | (-42, -42) | 0° | 70% | 2.0s | Collect ball 1 |
| **2** | (-24, -42) | 0° | 80% | 1.5s | Collect ball 2 |
| **3** | (-12, -48) | 0° | 60% | 1.5s | Collect ball 3 |
| **4** | (-48, -60) | 90° | 70% | 2.5s | Stop intake |
| **5** | (-60, -54) | 90° | 40% | 1.5s | Score balls |
| **Park** | (-60, -54) | 90° | - | 15s | Stop all |

---

## ⏱️ Time Budget Analysis

Based on your robot's calibrated speed (48" in 1.8s @ 60% = 26.7 in/s average):

### Movement Time Calculations:

**Segment 1:** Start → WP1
- Distance: ~25 inches
- Speed: 70%
- Estimated: **~2.0 seconds**

**Segment 2:** WP1 → WP2
- Distance: ~18 inches
- Speed: 80%
- Estimated: **~1.5 seconds**

**Segment 3:** WP2 → WP3
- Distance: ~13 inches
- Speed: 60%
- Estimated: **~1.5 seconds**

**Segment 4:** WP3 → WP4 (return)
- Distance: ~40 inches
- Speed: 70% backwards
- Estimated: **~2.5 seconds**

**Segment 5:** WP4 → Park
- Distance: ~14 inches
- Speed: 40%
- Estimated: **~1.5 seconds**

**Total Movement:** ~9 seconds

### Action Time:

- Intake ball collection (3x @ 0.3s each): **~1 second**
- Scoring outtake: **~2 seconds**
- Delays and transitions: **~1 second**

**Total Actions:** ~4 seconds

### Grand Total:
**Movement:** 9s
**Actions:** 4s
**Safety Margin:** 2s
**Total:** **~15 seconds** ✅

---

## 🔧 Robot-Specific Optimizations

This route is optimized for YOUR robot's characteristics:

1. **Speed Calibration:**
   - Based on your 48" in 1.8s test @ 60% speed
   - Uses 60-80% speeds for safety margin
   - Slows down near scoring zone (40%)

2. **Curve Smoothing:**
   - 0.3-0.5 smoothness factors
   - Prevents wheel slip on turns
   - Maintains momentum

3. **Heading Control:**
   - Explicit headings at key points
   - Ensures proper scoring orientation
   - Reduces turn time

---

## 🎮 Strategy Justification

### Why This Route?

**Conservative Approach:**
- 3 balls is reliable (not trying to maximize)
- Stays far from autonomous line (safety margin)
- Returns to known safe position (park barrier)

**Time Efficiency:**
- Sweeps in one direction (no backtracking)
- Minimal turning (smooth curves)
- Actions overlap with movement where possible

**Risk Mitigation:**
- Never crosses center (guaranteed legal)
- Starts and ends at barrier (rule compliance)
- Multiple balls for point redundancy

### Alternative Strategies (Not Recommended):

❌ **Aggressive 5-ball route:** Too risky, might timeout
❌ **Cross center line:** Violates your constraint
❌ **Skip park:** Loses potential points

---

## 📊 Expected Score Breakdown

Assuming Push Back scoring (estimated):

- **3 balls scored:** 3-6 points (depends on level)
- **Autonomous bonus:** 1-2 points
- **Park bonus:** 1-2 points (if touching barrier counts)

**Total Expected:** **5-10 points**

*Note: Adjust based on actual game manual scoring*

---

## 🔄 Tuning Recommendations

After first test run, adjust:

1. **If finishing early (< 14s):**
   - Add 4th ball at (-6, -54)
   - Increase speed to 90-100%
   - Add more scoring time

2. **If timing out (> 15s):**
   - Remove waypoint 3
   - Increase all speeds by 10%
   - Reduce scoring time to 1.5s

3. **If missing balls:**
   - Add 0.5s delays at collection points
   - Slow down collection segments
   - Check intake mechanism

4. **If drift issues:**
   - Check odometry calibration
   - Reduce speeds in turns
   - Add more curve smoothing

---

## 🚀 Implementation Steps

1. **Load the route:**
   ```bash
   python3 scripts/view_optimized_route.py
   ```

2. **Review in visualizer:**
   - Press 'L' to load optimized_15s_route.json
   - Press SPACE to simulate
   - Verify path stays on your half

3. **Export to code:**
   - Press 'W' to write to auton.cpp
   - Or copy from optimized_15s_auton.cpp

4. **Add to auton.hpp:**
   ```cpp
   void optimized15sCornerSweep();
   ```

5. **Call from main.cpp:**
   ```cpp
   auton.optimized15sCornerSweep();
   ```

6. **Test on robot:**
   - Build and upload
   - Test in controlled environment first
   - Adjust based on results

---

## 📈 Success Metrics

After running this route, you should see:

✅ Robot returns near starting position (< 6" error)
✅ All 3 balls collected successfully
✅ Completes in 14-16 seconds
✅ Never crosses center line
✅ Ends touching park barrier

If any metric fails, see "Tuning Recommendations" above.

---

## 🏆 Competition Day Checklist

Before running this autonomous:

- [ ] Field condition check (slippery vs sticky)
- [ ] Battery fully charged (voltage affects speed)
- [ ] Intake mechanism tested
- [ ] Odometry calibrated (IMU zeroed)
- [ ] Starting position marked on field
- [ ] Partner robot path confirmed (avoid collisions)
- [ ] Backup autonomous ready (in case this fails)

---

## 📝 Notes

- This route assumes standard Push Back field layout
- Adjust coordinates if field has variations
- Test extensively before competition
- Have a simpler fallback route ready
- Consider alliance partner's strategy

---

**Generated by AI-Powered Autonomous Route Optimizer**
🤖 Powered by Claude Code + Interactive Path Planner
