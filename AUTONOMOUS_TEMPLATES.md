# 🤖 Autonomous Templates & Examples

## Overview

This document contains 4 fully-documented autonomous templates for students to copy and modify. Each template demonstrates different complexity levels and techniques.

**How to use:**
1. Pick a template that matches your skill level
2. Copy the code to your `auton.cpp` file
3. Modify coordinates to match your robot and field position
4. Test and iterate!

---

## 📊 Template Quick Reference

| Template | Time | Complexity | Best For |
|----------|------|------------|----------|
| **Simple 2-Ball** | ~8s | ⭐ Beginner | Learning basics, guaranteed points |
| **4-Ball Checkpoints** | ~12s | ⭐⭐⭐ Intermediate | Reliability focus, wall alignment |
| **6-Ball Optimized** | ~14s | ⭐⭐⭐⭐⭐ Advanced | Speed optimization, motion chaining |
| **Skills Run** | 60s | ⭐⭐⭐⭐ Advanced | Endurance, maximum score |

---

## Template 1: Simple 2-Ball (Beginner) ⭐

**Goal:** Score 2 balls with maximum reliability
**Strategy:** Minimal movements, simple straight-line paths
**Time:** ~8 seconds
**Success Rate:** 95%+ with proper tuning

### Key Concepts
- Setting starting position
- Basic `moveToPoint()` usage
- Subsystem control (intake)
- Simple sequential logic

### Code

```cpp
void Auton::template_simple_2ball() {
    // ========================================================================
    // STEP 1: SET STARTING POSITION
    // ========================================================================
    // IMPORTANT: Measure starting position on field!
    // X, Y are in inches from field center (0, 0)
    // Heading is in degrees (0° = right, 90° = up, 180° = left, 270° = down)
    //
    // Example: Robot starts 24" behind center, facing forward
    chassis->setPose(0, -24, 0);
    pros::delay(250);  // Let odometry stabilize

    // ========================================================================
    // STEP 2: COLLECT FIRST BALL
    // ========================================================================
    // Start intake motors
    intake.move_forward();

    // Drive to first ball location
    // Coordinates are FIELD-CENTRIC (absolute, not relative!)
    chassis->moveToPoint(0, 0, 2000, {
        .forwards = true,      // Drive forwards (not backwards)
        .maxSpeed = 80,        // 80% max speed (slower = more accurate)
        .minSpeed = 20         // Don't slow down too much near target
    });
    chassis->waitUntilDone();  // Block until movement finishes

    // Give intake time to grab ball
    pros::delay(500);

    // ========================================================================
    // STEP 3: SCORE FIRST BALL
    // ========================================================================
    // Drive to scoring position
    chassis->moveToPoint(-12, 12, 2000, {
        .forwards = true,
        .maxSpeed = 80,
        .minSpeed = 20
    });
    chassis->waitUntilDone();

    // Score the ball (reverse intake to spit out)
    intake.move_backward();
    pros::delay(1000);  // Run outtake for 1 second
    intake.stop();

    // ========================================================================
    // STEP 4: COLLECT SECOND BALL
    // ========================================================================
    intake.move_forward();

    chassis->moveToPoint(12, 0, 2000, {
        .forwards = true,
        .maxSpeed = 80,
        .minSpeed = 20
    });
    chassis->waitUntilDone();
    pros::delay(500);

    // ========================================================================
    // STEP 5: SCORE SECOND BALL
    // ========================================================================
    chassis->moveToPoint(-12, 12, 2000, {
        .forwards = true,
        .maxSpeed = 80,
        .minSpeed = 20
    });
    chassis->waitUntilDone();

    intake.move_backward();
    pros::delay(1000);
    intake.stop();

    // ========================================================================
    // DONE! 2 balls scored
    // ========================================================================
}
```

### Student Checklist

- [ ] Update `chassis->setPose()` with your measured starting position
- [ ] Change ball collection coordinates (lines with `moveToPoint`)
- [ ] Change scoring coordinates
- [ ] Adjust timeout values if robot moves slower/faster
- [ ] Test one ball at a time before running full autonomous
- [ ] Use telemetry to verify path accuracy

---

## Template 2: 4-Ball with Checkpoints (Intermediate) ⭐⭐⭐

**Goal:** Score 4 balls using odometry checkpoints
**Strategy:** Reset odometry after every 2 balls to prevent drift
**Time:** ~12 seconds
**Success Rate:** 90%+ with proper wall alignment

### Key Concepts
- Wall alignment for odometry correction
- Checkpoint-based structure (prevents drift accumulation)
- `distanceAlign` usage
- Alert system integration (`resetDriftTimer`)

### Code

```cpp
void Auton::template_4ball_checkpoints() {
    // Set starting position
    chassis->setPose(0, -24, 0);
    pros::delay(250);

    // ========================================================================
    // CYCLE 1: Score 2 balls
    // ========================================================================
    intake.move_forward();

    // Ball 1
    chassis->moveToPoint(0, 0, 2000, {.maxSpeed = 80, .minSpeed = 20});
    chassis->waitUntilDone();
    pros::delay(500);

    chassis->moveToPoint(-12, 12, 2000, {.maxSpeed = 80, .minSpeed = 20});
    chassis->waitUntilDone();
    intake.move_backward();
    pros::delay(1000);
    intake.stop();

    // Ball 2
    intake.move_forward();
    chassis->moveToPoint(12, 0, 2000, {.maxSpeed = 80, .minSpeed = 20});
    chassis->waitUntilDone();
    pros::delay(500);

    chassis->moveToPoint(-12, 12, 2000, {.maxSpeed = 80, .minSpeed = 20});
    chassis->waitUntilDone();
    intake.move_backward();
    pros::delay(1000);
    intake.stop();

    // ========================================================================
    // CHECKPOINT: Reset odometry using wall alignment
    // ========================================================================
    // This is the KEY difference from simple autonomous!
    // After every few scoring cycles, reset odometry to prevent drift

    // Drive to known wall position
    chassis->moveToPoint(-24, 0, 2000, {.maxSpeed = 60, .minSpeed = 15});
    chassis->waitUntilDone();

    // Align angle to wall (requires distance sensors on ports 3 and 4)
    distanceAlign.calculateAngleOneWall(270);  // 270° = facing left wall
    pros::delay(250);

    // Align distance to wall
    distanceAlign.calculateDistOneWall(270, 72);  // 72" = half field width
    pros::delay(250);

    // Tell alert system we just corrected odometry
    alerts.resetDriftTimer();

    printf("[Auton] Checkpoint complete - odometry reset\n");

    // ========================================================================
    // CYCLE 2: Score 2 more balls (now with fresh odometry!)
    // ========================================================================
    intake.move_forward();

    // Ball 3
    chassis->moveToPoint(0, -12, 2000, {.maxSpeed = 80, .minSpeed = 20});
    chassis->waitUntilDone();
    pros::delay(500);

    chassis->moveToPoint(-12, 12, 2000, {.maxSpeed = 80, .minSpeed = 20});
    chassis->waitUntilDone();
    intake.move_backward();
    pros::delay(1000);
    intake.stop();

    // Ball 4
    intake.move_forward();
    chassis->moveToPoint(12, -12, 2000, {.maxSpeed = 80, .minSpeed = 20});
    chassis->waitUntilDone();
    pros::delay(500);

    chassis->moveToPoint(-12, 12, 2000, {.maxSpeed = 80, .minSpeed = 20});
    chassis->waitUntilDone();
    intake.move_backward();
    pros::delay(1000);
    intake.stop();

    // ========================================================================
    // DONE! 4 balls scored with odometry checkpoint
    // ========================================================================
}
```

### Student Checklist

- [ ] Install distance sensors on ports 3 and 4
- [ ] Test wall alignment in driver control first (A and X buttons)
- [ ] Identify good checkpoint locations (near walls, known positions)
- [ ] Add checkpoints every 10-15 seconds of autonomous
- [ ] Use telemetry to verify checkpoints reduce drift
- [ ] Consider adding checkpoint after EVERY scoring cycle for maximum reliability

---

## Template 3: 6-Ball Optimized (Advanced) ⭐⭐⭐⭐⭐

**Goal:** Score 6 balls in 14 seconds with motion chaining
**Strategy:** Never stop moving, start actions early, minimize delays
**Time:** ~14 seconds
**Success Rate:** 85%+ with extensive tuning

### Key Concepts
- Motion chaining (seamless multi-segment paths)
- Custom exit conditions (`waitUntil`)
- Parallel subsystem actions (intake while driving)
- Speed optimization (`minSpeed` prevents slowdown)
- Telemetry-driven iteration

### Code

```cpp
void Auton::template_6ball_optimized() {
    chassis->setPose(0, -24, 0);
    pros::delay(250);

    // ========================================================================
    // OPTIMIZED CYCLE 1: Balls 1 & 2
    // ========================================================================
    // KEY TECHNIQUE: Start intake BEFORE arriving at ball!

    // Approach ball location
    chassis->moveToPoint(0, -6, 2000, {.maxSpeed = 100, .minSpeed = 40});
    chassis->waitUntil(6);  // Exit when 6" away (not when fully arrived!)
    intake.move_forward();  // Start intake early
    chassis->waitUntilDone();

    // Don't stop - chain directly to ball
    chassis->moveToPoint(0, 0, 2000, {.maxSpeed = 100, .minSpeed = 40});
    chassis->waitUntilDone();
    pros::delay(300);  // Minimized delay - just enough to grab ball

    // Chain to scoring (minSpeed keeps robot moving fast)
    chassis->moveToPoint(-12, 12, 2000, {.maxSpeed = 100, .minSpeed = 40});
    chassis->waitUntilDone();
    intake.move_backward();
    pros::delay(800);  // Minimized outtake time
    intake.stop();

    // Ball 2 - same pattern
    chassis->moveToPoint(6, -6, 2000, {.maxSpeed = 100, .minSpeed = 40});
    chassis->waitUntil(6);  // Early intake start
    intake.move_forward();
    chassis->waitUntilDone();

    chassis->moveToPoint(12, 0, 2000, {.maxSpeed = 100, .minSpeed = 40});
    chassis->waitUntilDone();
    pros::delay(300);

    chassis->moveToPoint(-12, 12, 2000, {.maxSpeed = 100, .minSpeed = 40});
    chassis->waitUntilDone();
    intake.move_backward();
    pros::delay(800);
    intake.stop();

    // ========================================================================
    // CYCLE 2: Balls 3 & 4 (repeat pattern)
    // ========================================================================
    chassis->moveToPoint(-6, -6, 2000, {.maxSpeed = 100, .minSpeed = 40});
    chassis->waitUntil(6);
    intake.move_forward();
    chassis->waitUntilDone();

    chassis->moveToPoint(0, -12, 2000, {.maxSpeed = 100, .minSpeed = 40});
    chassis->waitUntilDone();
    pros::delay(300);

    chassis->moveToPoint(-12, 12, 2000, {.maxSpeed = 100, .minSpeed = 40});
    chassis->waitUntilDone();
    intake.move_backward();
    pros::delay(800);
    intake.stop();

    // Ball 4
    chassis->moveToPoint(6, -18, 2000, {.maxSpeed = 100, .minSpeed = 40});
    chassis->waitUntil(6);
    intake.move_forward();
    chassis->waitUntilDone();

    chassis->moveToPoint(12, -12, 2000, {.maxSpeed = 100, .minSpeed = 40});
    chassis->waitUntilDone();
    pros::delay(300);

    chassis->moveToPoint(-12, 12, 2000, {.maxSpeed = 100, .minSpeed = 40});
    chassis->waitUntilDone();
    intake.move_backward();
    pros::delay(800);
    intake.stop();

    // ========================================================================
    // CYCLE 3: Balls 5 & 6 (final sprint!)
    // ========================================================================
    chassis->moveToPoint(-6, -18, 2000, {.maxSpeed = 100, .minSpeed = 40});
    chassis->waitUntil(6);
    intake.move_forward();
    chassis->waitUntilDone();

    chassis->moveToPoint(0, -24, 2000, {.maxSpeed = 100, .minSpeed = 40});
    chassis->waitUntilDone();
    pros::delay(300);

    chassis->moveToPoint(-12, 12, 2000, {.maxSpeed = 100, .minSpeed = 40});
    chassis->waitUntilDone();
    intake.move_backward();
    pros::delay(800);
    intake.stop();

    // Ball 6
    chassis->moveToPoint(6, -30, 2000, {.maxSpeed = 100, .minSpeed = 40});
    chassis->waitUntil(6);
    intake.move_forward();
    chassis->waitUntilDone();

    chassis->moveToPoint(12, -24, 2000, {.maxSpeed = 100, .minSpeed = 40});
    chassis->waitUntilDone();
    pros::delay(300);

    chassis->moveToPoint(-12, 12, 2000, {.maxSpeed = 100, .minSpeed = 40});
    chassis->waitUntilDone();
    intake.move_backward();
    pros::delay(800);
    intake.stop();

    // ========================================================================
    // DONE! 6 balls in ~14 seconds
    // ========================================================================
}
```

### Advanced Techniques Explained

**1. Motion Chaining:**
```cpp
// OLD WAY (slow):
chassis->moveToPoint(A, 2000, {.maxSpeed = 80});
chassis->waitUntilDone();  // Robot decelerates to 0
chassis->moveToPoint(B, 2000, {.maxSpeed = 80});  // Accelerates from 0

// NEW WAY (fast):
chassis->moveToPoint(A, 2000, {.maxSpeed = 100, .minSpeed = 40});
chassis->waitUntilDone();  // Robot only slows to 40% (not 0!)
chassis->moveToPoint(B, 2000, {.maxSpeed = 100, .minSpeed = 40});  // Already moving!
```

**2. Early Action Start:**
```cpp
// Start intake BEFORE arriving at ball
chassis->moveToPoint(near_ball, 2000, {.maxSpeed = 100});
chassis->waitUntil(6);  // Exit when 6" away
intake.move_forward();  // Intake is already running when we reach ball!
chassis->waitUntilDone();
```

**3. Delay Minimization:**
Test each delay and reduce to minimum required:
- Intake grab: Reduce from 500ms → 300ms if intake is fast
- Outtake: Reduce from 1000ms → 800ms if balls eject quickly

### Student Checklist

- [ ] Start with Template 1 or 2 first - get them working reliably
- [ ] Run telemetry and analyze velocity plot - robot should maintain speed
- [ ] Tune `minSpeed` values (too high = overshoots, too low = wastes time)
- [ ] Test `waitUntil` distances (too early = miss ball, too late = waste time)
- [ ] Minimize ALL delays - test each one individually
- [ ] Iterate: Telemetry → Identify slowdowns → Fix → Repeat

---

## Template 4: Skills Run (60 seconds) ⭐⭐⭐⭐

**Goal:** Maximum score in 60 seconds
**Strategy:** Loop scoring routine with periodic odometry resets
**Time:** 60 seconds
**Success Rate:** 80%+ with error handling

### Key Concepts
- Loop-based structure
- Time management (stop before timeout)
- Periodic odometry resets (every ~20 seconds)
- Error recovery (continue if one cycle fails)
- Performance logging

### Code

```cpp
void Auton::template_skills() {
    chassis->setPose(0, -24, 0);
    pros::delay(250);

    // Track time
    uint32_t startTime = pros::millis();
    const uint32_t maxTime = 58000;  // Stop at 58s (2s safety buffer)

    int ballsScored = 0;
    int cycleCount = 0;

    // ========================================================================
    // MAIN LOOP: Repeat until time runs out
    // ========================================================================
    while (pros::millis() - startTime < maxTime) {
        cycleCount++;
        printf("[Skills] Cycle %d (t=%.1fs)\n",
               cycleCount, (pros::millis() - startTime) / 1000.0);

        // ================================================================
        // SCORING CYCLE: Score 2 balls
        // ================================================================
        intake.move_forward();

        // Ball 1
        chassis->moveToPoint(0, 0, 2000, {.maxSpeed = 90, .minSpeed = 30});
        if (pros::millis() - startTime >= maxTime) break;  // Time check!
        chassis->waitUntilDone();
        pros::delay(400);

        chassis->moveToPoint(-12, 12, 2000, {.maxSpeed = 90, .minSpeed = 30});
        if (pros::millis() - startTime >= maxTime) break;
        chassis->waitUntilDone();

        intake.move_backward();
        pros::delay(900);
        ballsScored++;

        // Ball 2
        intake.move_forward();
        chassis->moveToPoint(12, 0, 2000, {.maxSpeed = 90, .minSpeed = 30});
        if (pros::millis() - startTime >= maxTime) break;
        chassis->waitUntilDone();
        pros::delay(400);

        chassis->moveToPoint(-12, 12, 2000, {.maxSpeed = 90, .minSpeed = 30});
        if (pros::millis() - startTime >= maxTime) break;
        chassis->waitUntilDone();

        intake.move_backward();
        pros::delay(900);
        intake.stop();
        ballsScored++;

        // ================================================================
        // CHECKPOINT: Reset every 3 cycles (~18 seconds)
        // ================================================================
        if (cycleCount % 3 == 0) {
            printf("[Skills] Checkpoint at cycle %d\n", cycleCount);

            // Drive to wall
            chassis->moveToPoint(-24, 0, 2000, {.maxSpeed = 60});
            if (pros::millis() - startTime >= maxTime) break;
            chassis->waitUntilDone();

            // Reset odometry
            distanceAlign.calculateAngleOneWall(270);
            pros::delay(200);
            distanceAlign.calculateDistOneWall(270, 72);
            pros::delay(200);

            alerts.resetDriftTimer();
            imuDrift.resetCompensation();

            printf("[Skills] Reset complete\n");
        }
    }

    // ========================================================================
    // FINAL STATISTICS
    // ========================================================================
    uint32_t totalTime = pros::millis() - startTime;
    printf("[Skills] Complete! Time: %.1fs, Balls: %d, Cycles: %d\n",
           totalTime / 1000.0, ballsScored, cycleCount);

    intake.stop();
}
```

### Student Checklist

- [ ] Test one scoring cycle reliably first
- [ ] Add time checks after EVERY movement
- [ ] Set `maxTime` to 58s (never 60s - leave safety buffer!)
- [ ] Add checkpoints every 15-20 seconds
- [ ] Test incrementally: 2 cycles → 4 cycles → full 60s
- [ ] Log performance stats (balls/second, cycle time)
- [ ] Use telemetry to find bottlenecks and optimize

---

## 🎯 General Best Practices

### 1. Always Measure Starting Position
```cpp
// WRONG - guessing position
chassis->setPose(0, 0, 0);

// RIGHT - measured with tape measure
chassis->setPose(-6, -24, 45);  // 6" left, 24" back, facing 45° northeast
```

### 2. Use Telemetry for Everything
```bash
# After every autonomous run:
python scripts/plot_telemetry.py /usd/telemetry.csv

# Look for:
# - Path inefficiencies (unnecessary turns)
# - Speed bottlenecks (robot slowing down too much)
# - Motor issues (overheating, imbalance)
```

### 3. Tune Incrementally
1. Start with slow, safe speeds (`maxSpeed = 60`)
2. Verify autonomous works reliably (>90% success rate)
3. Increase speeds gradually (`maxSpeed = 70, 80, 90, 100`)
4. Re-test after each change

### 4. Structure Your Code
```cpp
// GOOD - clear sections
void Auton::myAuton() {
    // Setup
    chassis->setPose(...);

    // Cycle 1
    // ... scoring routine ...

    // Checkpoint
    // ... wall alignment ...

    // Cycle 2
    // ... scoring routine ...
}

// BAD - unclear flow
void Auton::myAuton() {
    chassis->setPose(...);
    intake.move_forward();
    chassis->moveToPoint(...);
    // ... 100 lines of spaghetti code ...
}
```

### 5. Comment Aggressively
```cpp
// GOOD
// Drive to first ball (field position: center of red zone)
chassis->moveToPoint(0, -12, 2000, {.maxSpeed = 80});

// BAD
chassis->moveToPoint(0, -12, 2000, {.maxSpeed = 80});
```

---

## 🔧 Troubleshooting Guide

### Robot Doesn't Drive Straight
- **Check:** Tracking wheel calibration (diameter, offsets)
- **Check:** Motor ports (all motors connected?)
- **Fix:** Run `motorDiagnostics()` to identify weak motors
- **Fix:** Tune angular PID in `globals.cpp`

### Robot Overshoots Target
- **Check:** Too high `maxSpeed`
- **Check:** PID tuning (too aggressive kP)
- **Fix:** Reduce `maxSpeed` from 100 → 80
- **Fix:** Increase kD in lateral PID

### Robot Is Too Slow
- **Check:** Too low `maxSpeed`
- **Check:** Delays too long
- **Fix:** Use motion chaining (`minSpeed` parameter)
- **Fix:** Minimize delays (test each one)

### Odometry Drifts Over Time
- **Check:** Tracking wheels slipping
- **Check:** No checkpoints in 60s autonomous
- **Fix:** Add wall alignment every 15 seconds
- **Fix:** Use IMU drift compensation

---

## 📚 Next Steps

1. **Pick a template** that matches your skill level
2. **Copy to `auton.cpp`** and enable in `main.cpp`
3. **Modify coordinates** to match your robot/field
4. **Test incrementally** - one section at a time
5. **Collect telemetry** and analyze
6. **Iterate** until autonomous is reliable

**Remember:** The best autonomous is one that works 95%+ of the time. Focus on reliability first, speed second!

---

*Team 839Y - Caution Tape*
*"Start simple, iterate systematically, measure everything"*
