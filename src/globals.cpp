#include "globals.hpp"
#include "pros/motors.h"

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
// 3. DRIVETRAIN
// ============================================================================

// Individual Motors (Blue Gearset = 600 RPM)
pros::Motor leftFrontMotor(LEFT_MOTOR_FRONT, pros::E_MOTOR_GEAR_BLUE);
pros::Motor leftMidMotor(LEFT_MOTOR_MID, pros::E_MOTOR_GEAR_BLUE);
pros::Motor leftRearMotor(LEFT_MOTOR_REAR, pros::E_MOTOR_GEAR_BLUE);
pros::Motor rightFrontMotor(RIGHT_MOTOR_FRONT, pros::E_MOTOR_GEAR_BLUE);
pros::Motor rightMidMotor(RIGHT_MOTOR_MID, pros::E_MOTOR_GEAR_BLUE);
pros::Motor rightRearMotor(RIGHT_MOTOR_REAR, pros::E_MOTOR_GEAR_BLUE);

// Motor Groups (for synchronized control)
pros::MotorGroup leftMotors({leftFrontMotor, leftMidMotor, leftRearMotor});
pros::MotorGroup rightMotors({rightFrontMotor, rightMidMotor, rightRearMotor});

// ============================================================================
// 4. SENSORS
// ============================================================================
pros::Imu inertial(INERTIAL);
pros::Rotation leftRotation(LEFT_ROTATION, true);     // Reversed
pros::Rotation rearRotation(REAR_ROTATION, true);     // Reversed

// ============================================================================
// 5. LEMLIB CONFIGURATION
// ============================================================================

// Tracking Wheels (for odometry)
// NOTE: Currently disabled due to port conflicts - using IMU-only tracking
// TODO: Fix port conflicts in globals.hpp, then enable in OdomSensors below
lemlib::TrackingWheel verticalTracking(&leftRotation, lemlib::Omniwheel::NEW_275, 0.0);
lemlib::TrackingWheel horizontalTracking(&rearRotation, lemlib::Omniwheel::NEW_2, 3.25);

// Drivetrain Configuration
lemlib::Drivetrain drivetrain {
    &leftMotors,                    // Left motor group
    &rightMotors,                   // Right motor group
    9.9,                            // Track width (inches) - distance between left/right wheels
    lemlib::Omniwheel::NEW_325,     // Wheel type: 3.25" omni wheels
    450,                            // Drivetrain RPM (with blue cartridge: 600 RPM * gear ratio)
    2                               // Horizontal drift correction (degrees)
};

// Odometry Sensors
// WARNING: Currently using IMU-only tracking (all tracking wheels set to nullptr)
// For better accuracy in competition:
//   1. Fix port conflicts in globals.hpp
//   2. Replace nullptr with &verticalTracking and &horizontalTracking
//   3. Test and tune tracking wheel offsets
lemlib::OdomSensors sensors(
    nullptr,        // Vertical tracking wheel 1 (left) - TODO: use &verticalTracking
    nullptr,        // Vertical tracking wheel 2 (right) - not used
    nullptr,        // Horizontal tracking wheel 1 (rear) - TODO: use &horizontalTracking
    nullptr,        // Horizontal tracking wheel 2 - not used
    &inertial       // IMU sensor (required)
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
lemlib::ControllerSettings lateralPID(
    10,     // kP - Proportional gain (aggression)
    0,      // kI - Integral gain (steady-state correction)
    1,      // kD - Derivative gain (dampening/smoothness)
    3,      // Anti-windup range
    1,      // Small error range (inches) - close enough to target
    100,    // Small error timeout (ms) - how long to stay in small error
    3,      // Large error range (inches) - far from target threshold
    500,    // Large error timeout (ms) - max time allowed in large error
    20      // Maximum acceleration/slew rate
);

// Angular PID (Turning)
lemlib::ControllerSettings angularPID(
    2.2,    // kP - Proportional gain (turn aggression)
    0,      // kI - Integral gain (usually 0 for turning)
    10,     // kD - Derivative gain (prevents overshoot on turns)
    3,      // Anti-windup range
    1,      // Small error range (degrees)
    100,    // Small error timeout (ms)
    3,      // Large error range (degrees)
    500,    // Large error timeout (ms)
    0       // Maximum acceleration/slew (0 = no slew on turns)
);

// Chassis Object (combines drivetrain, PID, and sensors)
lemlib::Chassis chassis(drivetrain, lateralPID, angularPID, sensors);

// ============================================================================
// 6. SUBSYSTEMS (Game-Specific)
// ============================================================================

// Intake Subsystem (High Stakes: color sorting)
pros::Motor intakeMotor1(INTAKE_MOTOR_1, pros::E_MOTOR_GEARSET_18);  // 18:1 gearset (red)
pros::Motor intakeMotor2(INTAKE_MOTOR_2, pros::E_MOTOR_GEARSET_06);  // 6:1 gearset (blue)
pros::Motor intakeMotor3(INTAKE_MOTOR_3, pros::E_MOTOR_GEARSET_18);  // 18:1 gearset (red)
subsystems::Intake intake(
    {intakeMotor1, intakeMotor2, intakeMotor3},     // Motor group
    INTAKE_PISTON_PORT,                              // Pneumatic piston
    INTAKE_COLOR_SENSOR_PORT,                        // Optical sensor for color sorting
    INTAKE_LIMIT_PORT                                // Limit switch
);

// Arm Subsystem (High Stakes: wall stake scoring)
pros::Motor rightArmMotor(RIGHT_ARM_MOTOR, pros::E_MOTOR_GEARSET_18);
pros::Motor leftArmMotor(LEFT_ARM_MOTOR, pros::E_MOTOR_GEARSET_18);
subsystems::Arm arm({leftArmMotor}, ARM_PISTON_PORT);

// Movement & Autonomous Control
subsystems::Movement movement(&chassis);
subsystems::Auton auton(&chassis);
subsystems::Selector selector(&intake, &auton);
subsystems::DistanceAlign distanceAlign(
    RIGHT_DISTANCE_PORT,    // Right distance sensor
    LEFT_DISTANCE_PORT,     // Left distance sensor
    6.5,                    // Sensor offset (inches)
    0.0                     // Angle offset (degrees)
);

// ============================================================================
// 7. GENERIC COMPONENTS (Game-Agnostic)
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
