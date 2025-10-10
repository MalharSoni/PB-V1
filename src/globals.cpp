#include "globals.hpp"
#include "pros/motors.h"
#include "robot/brain_ui.hpp"

// ============================================================================
// ROBOT HARDWARE CONFIGURATION
// ============================================================================
// This file contains all robot hardware object instantiations.
// Organized by category for easy navigation and modification.
//
// STRUCTURE:
// 1. State Variables (toggles, flags)
// 2. Controller
// 3. Drivetrain (motors, motor groups)
// 4. Sensors (IMU, rotation sensors)
// 5. LemLib Configuration (tracking wheels, drivetrain, PID, chassis)
// 6. Subsystems (Intake, Arm, Movement, Auton, etc.)
// 7. Generic Components (Pneumatics)
// ============================================================================

// ============================================================================
// 1. STATE VARIABLES
// ============================================================================
// Toggle variables for driver control state tracking
bool pistonToggle = false;
bool intakeToggle = false;
bool wallToggle = false;

// ============================================================================
// 2. CONTROLLER
// ============================================================================
pros::Controller master(pros::E_CONTROLLER_MASTER);

// ============================================================================
// 3. DRIVETRAIN (4-MOTOR DRIVE)
// ============================================================================

// Individual Motors (Blue Gearset = 600 RPM)
// NOTE: Motors 11 and 13 removed - no gearbox yet
pros::Motor leftFrontMotor(LEFT_MOTOR_FRONT, pros::E_MOTOR_GEAR_BLUE);
pros::Motor leftMidMotor(LEFT_MOTOR_MID, pros::E_MOTOR_GEAR_BLUE);
// pros::Motor leftRearMotor(LEFT_MOTOR_REAR, pros::E_MOTOR_GEAR_BLUE);  // REMOVED
pros::Motor rightFrontMotor(RIGHT_MOTOR_FRONT, pros::E_MOTOR_GEAR_BLUE);
pros::Motor rightMidMotor(RIGHT_MOTOR_MID, pros::E_MOTOR_GEAR_BLUE);
// pros::Motor rightRearMotor(RIGHT_MOTOR_REAR, pros::E_MOTOR_GEAR_BLUE);  // REMOVED

// Motor Groups (for synchronized control) - 2 motors per side
pros::MotorGroup leftMotors({leftFrontMotor, leftMidMotor});
pros::MotorGroup rightMotors({rightFrontMotor, rightMidMotor});

// ============================================================================
// 4. SENSORS
// ============================================================================
pros::Imu inertial(INERTIAL);
pros::Rotation leftRotation(LEFT_ROTATION, true);     // Vertical wheel (port 8) - REVERSED
pros::Rotation rearRotation(REAR_ROTATION, true);     // Horizontal wheel (port 7) - REVERSED

// ============================================================================
// 5. LEMLIB CONFIGURATION
// ============================================================================

// Tracking Wheels (for odometry)
// TANK DRIVE CONFIGURATION:
// - Vertical wheel: measures forward/backward movement
// - Horizontal wheel: measures ROTATION (not strafing - tank drives can't strafe!)
//
// Vertical wheel offset: Distance from robot center of rotation (left/right)
//   - Measured: 3/16" to the RIGHT of centerline = +0.1875" (positive = right side)
// Horizontal wheel offset: Distance from robot center of rotation (front/back)
//   - Measured: 4 inches rearwards = -4.0 offset (negative = behind center)
// NOTE: Tracking wheels measured at 2.865" diameter (calibrated from 48" test showing 2" overshoot)
// Robot reported 48" but physically traveled 50" -> 50/48 = 1.042 scale factor
// 2.75" (NEW_275) × 1.042 = 2.865" actual diameter
lemlib::TrackingWheel verticalTracking(&leftRotation, 2.865, 0.1875);
lemlib::TrackingWheel horizontalTracking(&rearRotation, 2.865, -4.0);

// Drivetrain Configuration
// HYBRID DRIVE: 4 omni wheels (outside) + 2 traction wheels (center)
lemlib::Drivetrain drivetrain {
    &leftMotors,                    // Left motor group
    &rightMotors,                   // Right motor group
    10.0,                           // Track width (inches) - MEASURED: exactly 10.0"
    lemlib::Omniwheel::NEW_325,     // Wheel type: 3.25" omni wheels
    450,                            // Drivetrain RPM (with blue cartridge: 600 RPM * gear ratio)
    8                               // Horizontal drift correction - 8 for hybrid drive with traction wheels
                                    // LemLib docs: use 2 for omni-only, 8 for traction wheels
};

// Odometry Sensors
// Tracking wheels enabled - ports 1 (left vertical) and 2 (rear horizontal)
// NOTE: Test and tune tracking wheel offsets during competition if needed
lemlib::OdomSensors sensors(
    &verticalTracking,   // Vertical tracking wheel (left) - port 1
    nullptr,             // Vertical tracking wheel 2 (right) - not used
    &horizontalTracking, // Horizontal tracking wheel (rear) - port 2
    nullptr,             // Horizontal tracking wheel 2 - not used
    &inertial            // IMU sensor (required) - port 10
);

// ============================================================================
// PID CONTROLLERS
// ============================================================================
// PID TUNING GUIDE FOR STUDENTS
// ============================================================================
// PID controllers determine how the robot moves and turns during autonomous.
// Each parameter affects robot behavior:
//
// PROPORTIONAL (kP):
//   - How aggressively robot corrects errors
//   - Too high = oscillation/shaking
//   - Too low = slow, won't reach target
//
// INTEGRAL (kI):
//   - Eliminates steady-state error over time
//   - Usually keep at 0 to prevent windup issues
//   - Only increase if robot consistently stops short of target
//
// DERIVATIVE (kD):
//   - Dampening to reduce overshoot
//   - Higher value = smoother but potentially slower
//   - Too high = sluggish response
//
// TO TUNE:
//   1. Start with kP only (set kI=0, kD=0)
//   2. Increase kP until robot oscillates, then reduce by 30%
//   3. Add kD to reduce oscillation
//   4. Only add kI if needed for steady-state error
// ============================================================================

// Lateral PID (Forward/Backward Movement)
// TUNING HISTORY:
//   kP=10, kD=1 → 50.04" (2.04" overshoot)
//   kP=8,  kD=3 → 26.41" (21.59" undershoot - too weak!)
//   kP=10, kD=2 → 48.08" but overshoots to 50-51" then dives back (bad path!)
//   kP=10, kD=5/10 → Still overshoots
//   kP=10, kD=3 (LemLib recommended) → Testing official defaults
lemlib::ControllerSettings lateralPID(
    10,     // kP - Proportional gain (LemLib default)
    0,      // kI - Integral gain
    3,      // kD - Derivative gain (LemLib default)
    3,      // Anti-windup range
    1,      // Small error range (inches) - LemLib default
    100,    // Small error timeout (ms)
    3,      // Large error range (inches) - LemLib default
    500,    // Large error timeout (ms)
    20      // Slew rate - LemLib default
);

// Angular PID (Turning)
// TUNING HISTORY:
//   kP=2.2, kD=10 → 4° overshoot on 90° turns (user report)
//   kP=2.2, kD=15 → 17° overshoot on 90° turns (telemetry data)
//   kP=2.2, kD=25 → Testing higher damping
lemlib::ControllerSettings angularPID(
    2.2,    // kP - Proportional gain (turn aggression)
    0,      // kI - Integral gain (usually 0 for turning)
    25,     // kD - Derivative gain (increased from 15 to reduce 17° overshoot)
    3,      // Anti-windup range
    2,      // Small error range (degrees) - relaxed from 1°
    100,    // Small error timeout (ms)
    5,      // Large error range (degrees) - relaxed from 3°
    500,    // Large error timeout (ms)
    0       // Maximum acceleration/slew (0 = no slew on turns)
);

// Chassis Object (combines drivetrain, PID, and sensors)
lemlib::Chassis chassis(drivetrain, lateralPID, angularPID, sensors);

// ============================================================================
// 6. SUBSYSTEMS (Game-Specific)
// ============================================================================

// Push Back Intake Subsystem - 3 Stage System
pros::Motor intakeStage1(INTAKE_STAGE_1, pros::E_MOTOR_GEARSET_18);  // 11W motor (or use 18:1)
pros::Motor intakeStage2(INTAKE_STAGE_2, pros::E_MOTOR_GEARSET_06);  // 5.5W motor (6:1 gearset)
pros::Motor intakeStage3(INTAKE_STAGE_3, pros::E_MOTOR_GEARSET_06);  // 5.5W motor (6:1 gearset)
subsystems::IntakePushback intake({intakeStage1, intakeStage2, intakeStage3});

// Old High Stakes Arm Subsystem (archived - no longer used)
// pros::Motor rightArmMotor(RIGHT_ARM_MOTOR, pros::E_MOTOR_GEARSET_18);
// pros::Motor leftArmMotor(LEFT_ARM_MOTOR, pros::E_MOTOR_GEARSET_18);
// subsystems::Arm arm({leftArmMotor}, ARM_PISTON_PORT);

// Movement & Autonomous Control
subsystems::Movement movement(&chassis);
subsystems::Auton auton(&chassis);
// subsystems::Selector selector(&intake, &auton);  // Old High Stakes (archived)
subsystems::DistanceAlign distanceAlign(
    RIGHT_DISTANCE_PORT,    // Right distance sensor
    LEFT_DISTANCE_PORT,     // Left distance sensor
    6.5,                    // Sensor offset (inches)
    0.0                     // Angle offset (degrees)
);

// Brain Screen UI (LVGL)
subsystems::BrainUI brainUI(&auton);

// ============================================================================
// 7. RELIABILITY & DIAGNOSTICS SUBSYSTEMS
// ============================================================================
subsystems::Telemetry telemetry;  // CSV telemetry logger
subsystems::Alerts alerts;         // Real-time controller alerts

// ============================================================================
// 8. GENERIC COMPONENTS (Game-Agnostic)
// ============================================================================
// These use universal lib:: classes and can be renamed for any game.
//
// STUDENTS: For new seasons, just rename these variables to match your game!
//
// High Stakes (current):
lib::Pneumatic clamp(CLAMP_PORT);      // Mobile goal clamp
lib::Pneumatic doinker(DOINKER_PORT);  // Doinker mechanism
//
// Future season examples:
//   lib::Pneumatic wings(WINGS_PORT);
//   lib::Pneumatic blocker(BLOCKER_PORT);
//   lib::Pneumatic lift(LIFT_PORT);
// ============================================================================
