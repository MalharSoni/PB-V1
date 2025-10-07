#include "robot/auton.hpp"
#include "globals.hpp"
#include "lemlib/chassis/chassis.hpp"
#include "pros/rtos.hpp"

/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                      AUTONOMOUS PROGRAMMING GUIDE                            ║
║                       For Students & Programmers                             ║
╚══════════════════════════════════════════════════════════════════════════════╝

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
COORDINATE SYSTEM & FIELD SETUP
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  Field Size: 144" × 144" (12 feet × 12 feet)
  Origin:     Center of field = (0, 0)
  X-axis:     Positive = RIGHT, Negative = LEFT (from your alliance)
  Y-axis:     Positive = FORWARD, Negative = BACKWARD
  Units:      INCHES
  Heading:    0° = facing along +X axis
              Positive rotation = COUNTER-CLOCKWISE (CCW)
              90° = +Y, 180° = -X, 270°/-90° = -Y

  Valid coordinates: X,Y ∈ [-72, +72] inches

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
MOVEMENT COMMANDS (LemLib Chassis)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

1. SET STARTING POSITION
   ────────────────────────────────────────────────────────────────────────
   chassis->setPose(x, y, heading);

   Example: chassis->setPose(-9, -46.1, 150);
            // Start at (-9, -46.1) inches, facing 150°

2. MOVE TO FIELD COORDINATE
   ────────────────────────────────────────────────────────────────────────
   chassis->moveToPoint(x, y, timeout_ms, {options});

   Options:
     .forwards = false     // Drive backwards to point
     .maxSpeed = 100       // Speed (0-127), default varies by PID

   Examples:
     chassis->moveToPoint(24, -36, 2000);
     // Move to (24, -36) with 2 second timeout

     chassis->moveToPoint(20, -21.5, 1500, {.forwards = false, .maxSpeed = 60});
     // Drive backwards to point at max 60 speed

3. TURN TO FACE A POINT
   ────────────────────────────────────────────────────────────────────────
   chassis->turnToPoint(x, y, timeout_ms, {options}, async);

   Options:
     .forwards = true/false   // Face forwards or backwards to point
     .maxSpeed = 100          // Turn speed
   async: false = blocking, true = non-blocking

   Example: chassis->turnToPoint(48, -17, 900, {.maxSpeed = 70}, false);

4. TURN TO ABSOLUTE HEADING
   ────────────────────────────────────────────────────────────────────────
   chassis->turnToHeading(degrees, timeout_ms);

   Example: chassis->turnToHeading(90, 500);
            // Turn to face 90° (pointing in +Y direction)

5. WAIT FOR MOVEMENT COMPLETION
   ────────────────────────────────────────────────────────────────────────
   chassis->waitUntil(distance_inches);
   // Continue when robot is within X inches of target

   chassis->waitUntilDone();
   // Block until robot completely stops

   Examples:
     chassis->moveToPoint(24, 12, 2000);
     chassis->waitUntil(5);     // Continue when within 5 inches
     // Start next action while still moving!

     chassis->moveToPoint(0, 0, 3000);
     chassis->waitUntilDone();  // Wait for complete stop

6. GET CURRENT POSITION
   ────────────────────────────────────────────────────────────────────────
   lemlib::Pose pose = chassis->getPose();
   // Returns: pose.x, pose.y, pose.theta

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
HELPER MOVEMENT COMMANDS (Relative to Current Heading)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

7. MOVE FORWARD (relative to robot's current heading)
   ────────────────────────────────────────────────────────────────────────
   go_forward(distance_inches, timeout_ms, speed);

   Example: go_forward(12, 1000, 100);
            // Move 12" forward in direction robot is facing

8. MOVE BACKWARD (relative to robot's current heading)
   ────────────────────────────────────────────────────────────────────────
   go_backward(distance_inches, timeout_ms);

   Example: go_backward(6, 800);
            // Move 6" backward from current position

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
SUBSYSTEM COMMANDS
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

INTAKE SUBSYSTEM:
────────────────────────────────────────────────────────────────────────────
  intake.set_target_color(DONUT_COLOR::RED);     // Set alliance (RED or BLUE)
  intake.set_target_color(DONUT_COLOR::BLUE);

  intake.startColourSort();   // Start intake with automatic color rejection
  intake.activate();          // Run intake motors only (no color sorting)
  intake.deactivate();        // Stop intake completely

  intake.piston_out();        // Extend intake piston
  intake.piston_in();         // Retract intake piston

ARM SUBSYSTEM:
────────────────────────────────────────────────────────────────────────────
  arm.goToPickup();           // Move to pickup position (get ring from intake)
  arm.goToIdle();             // Move to idle/safe position
  arm.readyScore();           // Ready to score on wall stake
  arm.score();                // Score ring on wall stake (after readyScore)
  arm.readyScoreAlliance();   // Ready to score on alliance stake
  arm.scoreAlliance();        // Score on alliance stake

CLAMP / PNEUMATIC SUBSYSTEM:
────────────────────────────────────────────────────────────────────────────
  clamp.extend();             // Extend clamp (grab mobile goal)
  clamp.retract();            // Retract clamp (release mobile goal)
  clamp.toggle();             // Toggle between extended/retracted

  // Alternative method names (all do the same thing):
  clamp.activate();  / clamp.deactivate();
  clamp.open();      / clamp.close();
  clamp.deploy();    / clamp.stow();

DOINKER / PNEUMATIC SUBSYSTEM:
────────────────────────────────────────────────────────────────────────────
  doinker.extend();           // Extend doinker mechanism
  doinker.retract();          // Retract doinker mechanism
  doinker.toggle();           // Toggle between extended/retracted

  // Alternative method names (all do the same thing):
  doinker.activate();  / doinker.deactivate();
  doinker.open();      / doinker.close();
  doinker.deploy();    / doinker.stow();

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
TIMING & DELAYS
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  pros::delay(milliseconds);

  Example: pros::delay(500);   // Wait 0.5 seconds
           pros::delay(1000);  // Wait 1 second

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
PROGRAMMING TIPS & BEST PRACTICES
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  ✓ ALWAYS set starting pose at the beginning of your autonomous routine
  ✓ Use waitUntil() to start next action while still moving (saves time!)
  ✓ Use waitUntilDone() when you need robot fully stopped
  ✓ Coordinate subsystems with chassis movement:
      Example: Start intake BEFORE moving to ring location
  ✓ For RED/BLUE alliance mirroring:
      - Negate X coordinates (flip left/right)
      - Negate headings (flip angles)
  ✓ Timeouts are in MILLISECONDS (1000 ms = 1 second)
  ✓ Test small sections at a time - don't write entire auton before testing!
  ✓ Use delays to give mechanisms time to complete actions
  ✓ Comment your code so teammates understand what each section does

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
EXAMPLE AUTONOMOUS ROUTINE
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void Auton::exampleRoutine() {
    // 1. Set starting position (REQUIRED!)
    chassis->setPose(0, -60, 0);  // Start at (0, -60), facing 0°

    // 2. Move backward and grab mobile goal
    chassis->moveToPoint(0, -24, 2000, {.forwards = false, .maxSpeed = 60});
    chassis->waitUntil(5);  // Continue when within 5 inches
    clamp.extend();         // Grab the goal
    pros::delay(200);       // Give clamp time to close

    // 3. Start intake and collect first ring
    intake.set_target_color(DONUT_COLOR::RED);  // Set alliance color
    intake.startColourSort();                    // Start color sorting
    chassis->turnToPoint(12, -12, 800);         // Face the ring
    chassis->moveToPoint(12, -12, 1500);        // Drive to ring
    pros::delay(400);                           // Let intake grab ring

    // 4. Collect second ring
    chassis->turnToPoint(24, -12, 800);
    chassis->moveToPoint(24, -12, 1500);
    pros::delay(400);

    // 5. Stop intake and return to start zone
    intake.deactivate();
    chassis->moveToPoint(0, 0, 3000, {.forwards = false});
    chassis->waitUntilDone();  // Wait for complete stop
}

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
COMMON MISTAKES TO AVOID
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  ✗ Forgetting to set starting pose → Robot thinks it's at (0,0,0)
  ✗ Not giving mechanisms time to complete → Use pros::delay()
  ✗ Timeout too short → Robot stops before reaching target
  ✗ Wrong units → Remember: inches and degrees, not feet or radians
  ✗ Forgetting waitUntilDone() → Next command starts while robot still moving
  ✗ Wrong alliance color in intake → Will reject your own rings!

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

                        YOUR AUTONOMOUS ROUTINES START BELOW

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
*/

#define PI 3.14159265
namespace subsystems {

Auton::Auton(lemlib::Chassis *ichassis)
    : chassis(ichassis), selected_auton(AUTON_ROUTINE::TEST) {}

void Auton::go_forward(float distance, float time, float spd=127) {
  lemlib::Pose pose = chassis->getPose();
  float currX = pose.x;
  float currY = pose.y;
  float currTheta = pose.theta; // Angle in degrees

  // Normalize theta to [0, 360)
  while (currTheta < 0) {
    currTheta += 360;
  }
  while (currTheta >= 360) {
    currTheta -= 360;
  }

  // Compute correct deltaX and deltaY based on the robot's heading system
  float deltaX =
      distance * sin(currTheta * M_PI / 180.0); // X is based on sin()
  float deltaY =
      distance * cos(currTheta * M_PI / 180.0); // Y is based on cos()

  // Calculate the new position
  float newX = currX + deltaX;
  float newY = currY + deltaY;

  // Move the robot to the new position
  chassis->moveToPoint(newX, newY, time, {.maxSpeed=spd});
}
void Auton::go_backward(float distance, float time) {

  lemlib::Pose pose = chassis->getPose();
  float currX = pose.x;
  float currY = pose.y;
  float currTheta = pose.theta; // Angle in degrees

  // Normalize theta to [0, 360)
  while (currTheta < 0) {
    currTheta += 360;
  }
  while (currTheta >= 360) {
    currTheta -= 360;
  }

  // Compute correct deltaX and deltaY based on the robot's heading system
  float deltaX =
      distance * sin(currTheta * M_PI / 180.0); // X is based on sin()
  float deltaY =
      distance * cos(currTheta * M_PI / 180.0); // Y is based on cos()

  // Calculate the new position
  float newX = currX - deltaX;
  float newY = currY - deltaY;

  // Move the robot to the new position
  chassis->moveToPoint(newX, newY, time);
}

// ============================================================================
// PUSH BACK AUTONOMOUS ROUTINES
// ============================================================================

void Auton::pushBackSimple() {
    // Simple autonomous: Intake balls and score

    // 1. Set starting position (x, y, heading in degrees)
    // Example: Starting at (-12, -60, 0) - back left of field, facing forward
    chassis->setPose(0.75, -49.25, 0);

    // 2. Start intake to store balls
    intake.store();

    // 3. Move to first point to collect balls (x, y, timeout_ms)
    chassis->moveToPoint(23, -25, 2000);  
    chassis->waitUntilDone();  // Wait until completely stopped
    pros::delay(500);  // pause

    // 4. Move to second point while still intaking
    chassis->moveToPoint(43, -8, 2000);  // Move to center area
    chassis->waitUntilDone();  // Wait until completely stopped
    pros::delay(500);  // pause

    // 5. Stop intake
    intake.stopAll();
    pros::delay(200);  // Brief pause

    // 6. Score balls at level 2
    chassis->moveToPoint(12, -12, 1500, {.forwards = false, .maxSpeed = 60});
    chassis->turnToPoint(62, -72, 1500, {.maxSpeed = 70}, false);
    intake.scoreLevel2();
    pros::delay(1000);  // Run outtake for 1 second

    // 7. Stop scoring
    intake.stopAll();
}

// ============================================================================
// OLD HIGH STAKES AUTONOMOUS ROUTINES (ARCHIVED)
// ============================================================================
// These autonomous routines are specific to VEX High Stakes game.
// They use the old intake (color sorting) and arm subsystems that have been
// archived. Preserved here for reference when creating new Push Back autons.
// ============================================================================

/*
void Auton::elimSAFEBLUE() {
    intake.set_target_color(DONUT_COLOR::BLUE);
  chassis->setPose(9, -46.1, -150);
  go_forward(7, 1000);
  // chassis->moveToPoint(-8, -53, 500);
  chassis->waitUntil(3);

  // alliance stake
  arm.readyScore();
  pros::delay(600);

  // clamp mogo
  chassis->moveToPoint(20, -21.5, 1500, {.forwards = false, .maxSpeed = 60});
  chassis->waitUntil(7);
  arm.goToIdle();
  pros::delay(670);
  clamp.extend();

  // first ring in the ring stack
  intake.startColourSort();
  chassis->turnToPoint(34, -7, 900, {.maxSpeed = 70}, false);
  chassis->moveToPoint(34, -7, 1500, {.maxSpeed = 120}, false);
  pros::delay(50);
  go_backward(3, 300);
  chassis->waitUntilDone();

  // second ring in the ring stack
  chassis->turnToPoint(51, -3.75, 300, {.maxSpeed = 100});

  chassis->moveToPoint(51, -3.75, 1600, {.maxSpeed = 100});
  pros::delay(600);

  // lone ring off of middle line
//   intake.deactivate();
  chassis->turnToPoint(48, -17, 900, {.maxSpeed = 70}, false);
//   intake.startColourSort();
  chassis->moveToPoint(48, -17, 1000, {.maxSpeed = 110}, false);
  pros::delay(100);

  // get corner ring
  chassis->turnToPoint(62, -58, 700, {.maxSpeed = 85});
  chassis->moveToPoint(51, -39, 600, {.maxSpeed = 110});
  chassis->moveToPoint(63, -59, 1200, {.maxSpeed = 80}, false);
  pros::delay(50);

  // get dropped ring
  chassis->moveToPoint(42, -30, 800, {.forwards = false, .maxSpeed = 120}, false);
  chassis->turnToPoint(2, -31, 900, {.forwards = true, .maxSpeed = 100}, false);
  chassis->moveToPoint(2, -31, 1200, {.maxSpeed = 120}, false);

  // go to corner
  intake.deactivate();
  chassis->turnToPoint(-61, -31, 800, {.forwards = false}, false);
  intake.startColourSort();
  chassis->moveToPoint(-61, -54, 5000, {.forwards = false}, false);
  chassis->turnToHeading(0, 500);
}

void Auton::elimSAFERED() {
  intake.set_target_color(DONUT_COLOR::RED);
  chassis->setPose(-9, -46.1, 150);
  go_forward(7, 1000);
  intake.startColourSort();
  pros::delay(50);
  intake.deactivate();
  // chassis->moveToPoint(-8, -53, 500);
  chassis->waitUntil(3);

  // alliance stake
  arm.readyScore();
  pros::delay(600);

  // clamp mogo
  chassis->moveToPoint(-20, -21.5, 1500, {.forwards = false, .maxSpeed = 60});
  chassis->waitUntil(7);
  arm.goToIdle();
  pros::delay(670);
  clamp.extend();

  // first ring in the ring stack
  intake.startColourSort();
  chassis->turnToPoint(-34, -7, 900, {.maxSpeed = 70}, false);
  chassis->moveToPoint(-34, -7, 1500, {.maxSpeed = 120}, false);
  pros::delay(50);
  go_backward(5, 800);
  chassis->waitUntilDone();

  // second ring in the ring stack
  chassis->moveToPoint(-51.5, -4, 1600, {.maxSpeed = 100});
  pros::delay(600);

  // lone ring off of middle line
  intake.deactivate();
  chassis->turnToPoint(-48, -17, 900, {.maxSpeed = 70}, false);
  intake.startColourSort();
  chassis->moveToPoint(-48, -17, 1000, {.maxSpeed = 110}, false);
  pros::delay(100);

  // get corner ring
  chassis->turnToPoint(-62, -58, 700, {.maxSpeed = 70});
  chassis->moveToPoint(-51, -39, 600, {.maxSpeed = 110});
  chassis->moveToPoint(-62, -58, 1000, {.maxSpeed = 60}, false);
  pros::delay(50);

  // get dropped ring
  chassis->moveToPoint(-42, -24, 800, {.forwards = false, .maxSpeed = 120}, false);
  chassis->turnToPoint(-2, -31, 900, {.forwards = true, .maxSpeed = 120}, false);
  chassis->moveToPoint(-2, -31, 1200, {.maxSpeed = 120}, false);

  // go to corner
  intake.deactivate();
  chassis->turnToPoint(61, -31, 800, {.forwards = false}, false);
  intake.startColourSort();
  chassis->moveToPoint(61, -64, 5000, {.forwards = false}, false);
}

void Auton::swpRED() { //done test on practice field
  intake.set_target_color(DONUT_COLOR::RED);
  chassis->setPose(-9, -46.1, 150);
  go_forward(7, 1000);
  // chassis->moveToPoint(-8, -53, 500);
  chassis->waitUntil(2);

  // alliance stake
  arm.readyScore();
  pros::delay(550);
  chassis->moveToPoint(-20, -21.5, 2000, {.forwards = false, .maxSpeed = 60});
  chassis->waitUntil(7);
  arm.goToIdle();
  pros::delay(670);
  clamp.extend();
  intake.startColourSort();
  chassis->turnToPoint(-34, -7, 900, {.maxSpeed = 70});
  chassis->waitUntilDone();

  // first ring in the ring stack
  chassis->moveToPoint(-34, -7, 1500, {.maxSpeed = 100});
  pros::delay(500);
  chassis->waitUntilDone();
  go_backward(4, 800);
  chassis->waitUntilDone();

  // second ring in the ring stack
  chassis->moveToPoint(-51.5, -3, 1600, {.maxSpeed = 100});
  chassis->turnToHeading(-160, 900);
  chassis->moveToPoint(2, -32, 1300, {.maxSpeed = 110});
  pros::delay(700);

  // cross to push bot off line and grab MOGO
  chassis->moveToPoint(34, -37, 1600, {.maxSpeed = 100});
  chassis->waitUntil(19);
  clamp.retract();
  intake.deactivate();
  pros::delay(700);
  chassis->moveToPoint(24, -36, 1600, {.forwards = false, .maxSpeed = 100});
  chassis->turnToHeading(180, 800);
  chassis->moveToPoint(25, -20, 1000, {.forwards = false, .maxSpeed = 80});
  pros::delay(550);
  clamp.extend();
  intake.startColourSort();
  chassis->turnToPoint(44, -22, 500, {.maxSpeed = 110});

  // get final ring and touch
  intake.startColourSort();
  chassis->moveToPoint(48, -25, 1200, {.maxSpeed = 110});
  chassis->moveToPoint(12, -8, 5000, {.forwards = false, .maxSpeed = 100});
}

void Auton::swpBLUE() { //done test on practice field
  intake.set_target_color(DONUT_COLOR::BLUE);
  chassis->setPose(9, -46.1, -150);

  intake.activate();
  pros::delay(50);
  intake.deactivate();
  go_forward(7, 1000);
  // chassis->moveToPoint(8, -53, 500);
  chassis->waitUntil(2);

  // alliance stake
  arm.readyScore();
  pros::delay(550);
  chassis->moveToPoint(20, -21.5, 2000, {.forwards = false, .maxSpeed = 60});
  chassis->waitUntil(7);
  arm.goToIdle();
  pros::delay(600);
  clamp.extend();
  intake.startColourSort();
  chassis->turnToPoint(34, -7, 900, {.maxSpeed = 70});
  chassis->waitUntilDone();

  // first ring in the ring stack
  chassis->moveToPoint(34, -7, 1500, {.maxSpeed = 100});
  pros::delay(500);
  chassis->waitUntilDone();
  go_backward(4, 800);
  chassis->waitUntilDone();

  // second ring in the ring stack
  chassis->moveToPoint(50, -2, 1600, {.maxSpeed = 100});
  chassis->turnToHeading(160, 900);
  chassis->moveToPoint(2, -32, 1300, {.maxSpeed = 110});
  pros::delay(700);

  //cross to push bot off line and grab MOGO
  chassis->moveToPoint(-34, -37, 1600, {.maxSpeed = 100});
  chassis->waitUntil(19);
  clamp.retract();
  intake.deactivate();
  pros::delay(700);
  chassis->moveToPoint(-24, -36, 1600, {.forwards = false, .maxSpeed = 100});
  chassis->turnToHeading(-180, 800);
  chassis->moveToPoint(-25, -20, 1000, {.forwards = false, .maxSpeed = 80});
  pros::delay(550);
  clamp.extend();
  intake.startColourSort();
  chassis->turnToPoint(-44, -22, 500, {.maxSpeed = 110});

  // get final ring and touch
  intake.startColourSort();
  chassis->moveToPoint(-48, -25, 1200, {.maxSpeed = 110});
  chassis->moveToPoint(-12, -8, 5000, {.forwards = false, .maxSpeed = 100});

  // //new toucvh
  // chassis->turnToPoint(0, -24, 500, {.maxSpeed = 110});
  // chassis->moveToPoint(0, -24, 10000, {.maxSpeed = 50});
}
*/

void Auton::run_auton(AUTON_ROUTINE routine) {
  switch (routine) {
  case AUTON_ROUTINE::RED_RUSH:
    break;
  case AUTON_ROUTINE::RED_HIGH_SCORE:
    break;
  case AUTON_ROUTINE::BLUE_RUSH:
    break;
  case AUTON_ROUTINE::BLUE_HIGH_SCORE:
    break;
  case AUTON_ROUTINE::TEST:
    break;
  case AUTON_ROUTINE::SKILLS:
    skills();
    break;
  case AUTON_ROUTINE::NONE:
    break;
  }
}

void Auton::set_selected_auton(AUTON_ROUTINE routine) {
  selected_auton = routine;
}
AUTON_ROUTINE Auton::get_selected_auton() { return selected_auton; }

// ============================================================================
// STUB IMPLEMENTATIONS FOR MISSING ROUTINES
// Students: Implement these when you're ready to add more autonomous routines
// ============================================================================

void Auton::ringRushRed() {
    // TODO: Implement ring rush autonomous for RED alliance
    // Strategy: Use doinker and intake to quickly collect rings
}

void Auton::ringRushBlue() {
    // TODO: Implement ring rush autonomous for BLUE alliance
    // Strategy: Use doinker and intake to quickly collect rings
}

void Auton::mogoRushRed() {
    // TODO: Implement mobile goal rush for RED alliance
    // Strategy: Quickly grab mobile goal and score rings
}

void Auton::mogoRushBlue() {
    // TODO: Implement mobile goal rush for BLUE alliance
    // Strategy: Quickly grab mobile goal and score rings
}

void Auton::safeMogoRed() {
    // TODO: Implement safe mobile goal autonomous for RED alliance
    // Strategy: Grab second mobile goal and end under ladder
}

void Auton::safeMogoBlue() {
    // TODO: Implement safe mobile goal autonomous for BLUE alliance
    // Strategy: Grab second mobile goal and end under ladder
}

void Auton::interferenceRed() {
    // TODO: Implement interference autonomous for RED alliance
    // Strategy: Prevent opponent alliance from scoring AWP
}

void Auton::interferneceBlue() {
    // TODO: Implement interference autonomous for BLUE alliance
    // Strategy: Prevent opponent alliance from scoring AWP
}

void Auton::skills() {
    // TODO: Implement skills routine
    // Target: 65+ points
    // Strategy: Maximize scoring in 60 seconds
}

} // namespace subsystems