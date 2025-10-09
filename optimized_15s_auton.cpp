void Auton::optimized15sCornerSweep() {
    // ========================================================================
    // OPTIMIZED 15-SECOND CORNER SWEEP - AI Generated Route
    // ========================================================================
    // Strategy: Conservative 3-ball collection staying on own half
    // - Starts at (-60, -60) touching park barrier
    // - Sweeps along back wall collecting balls
    // - Returns to park barrier to finish
    // - Total estimated time: ~15 seconds
    // - Respects autonomous line (never crosses center)
    // ========================================================================

    // Set starting position - back left corner, touching park barrier
    chassis->setPose(-60.0, -60.0, 45);
    pros::delay(200);

    // ========================================================================
    // PHASE 1: START INTAKE AND APPROACH FIRST BALL
    // ========================================================================
    // Start intake system
    intake.store();  // Begin collecting balls
    pros::delay(100);

    // Move to first ball near corner (2 seconds)
    // Waypoint 1: (-42, -42) - First ball collection
    chassis->moveToPoint(-42.0, -42.0, 2000, {.maxSpeed = 70});
    chassis->turnToHeading(0, 800);  // Face along X-axis
    chassis->waitUntilDone();
    pros::delay(300);  // Allow intake to grab ball

    // ========================================================================
    // PHASE 2: SWEEP ALONG BACK WALL
    // ========================================================================
    // Waypoint 2: (-24, -42) - Second ball (1.5 seconds)
    chassis->moveToPoint(-24.0, -42.0, 1500, {.maxSpeed = 80});
    chassis->waitUntilDone();
    pros::delay(300);  // Allow intake to grab ball

    // Waypoint 3: (-12, -48) - Third ball near center line (1.5 seconds)
    // IMPORTANT: Stays at X=-12 to respect autonomous line at X=0
    chassis->moveToPoint(-12.0, -48.0, 1500, {.maxSpeed = 60});
    chassis->waitUntilDone();
    pros::delay(300);  // Allow intake to grab ball

    // ========================================================================
    // PHASE 3: RETURN TO SCORING ZONE
    // ========================================================================
    // Stop intake, prepare to score
    intake.stopAll();
    pros::delay(100);

    // Move back to scoring position (2.5 seconds)
    // Waypoint 4: (-48, -60) - Near scoring zone
    chassis->moveToPoint(-48.0, -60.0, 2500, {.maxSpeed = 70, .forwards = false});
    chassis->turnToHeading(90, 1000);  // Face scoring direction
    chassis->waitUntilDone();

    // ========================================================================
    // PHASE 4: SCORE BALLS
    // ========================================================================
    // Score at level 2 (adjust based on your mechanism)
    intake.scoreLevel2();
    pros::delay(2000);  // Run outtake for 2 seconds

    // ========================================================================
    // PHASE 5: FINAL PARK
    // ========================================================================
    // Stop scoring
    intake.stopAll();

    // Move to final park position touching barrier (1.5 seconds)
    // Waypoint 5: (-60, -54) - Touch park barrier
    chassis->moveToPoint(-60.0, -54.0, 1500, {.maxSpeed = 40, .forwards = false});
    chassis->waitUntilDone();

    // ========================================================================
    // AUTONOMOUS COMPLETE
    // ========================================================================
    // Expected results:
    // - 3 balls collected and scored
    // - Stayed on own half (respected autonomous line)
    // - Parked touching barrier
    // - Total time: ~14-15 seconds
    // ========================================================================
}
