#include "globals.hpp"
#include "pros/motors.h"

// ============================================================================
// GLOBAL VARIABLE INITIALIZATIONS
// ============================================================================

// Toggle variables for driver control state tracking
bool pistonToggle = false;
bool intakeToggle = false;
bool wallToggle = false;

// ============================================================================
// HARDWARE INITIALIZATION
// ============================================================================

// ----------------------------------------------------------------------------
// Controller
// ----------------------------------------------------------------------------
pros::Controller master(pros::E_CONTROLLER_MASTER);

// ----------------------------------------------------------------------------
// Drivetrain Motors (Blue Gearset = 600 RPM)
// ----------------------------------------------------------------------------
pros::Motor leftFrontMotor(LEFT_MOTOR_FRONT, pros::E_MOTOR_GEAR_BLUE);
pros::Motor leftMidMotor(LEFT_MOTOR_MID, pros::E_MOTOR_GEAR_BLUE);
pros::Motor leftRearMotor(LEFT_MOTOR_REAR, pros::E_MOTOR_GEAR_BLUE);
pros::Motor rightFrontMotor(RIGHT_MOTOR_FRONT, pros::E_MOTOR_GEAR_BLUE);
pros::Motor rightMidMotor(RIGHT_MOTOR_MID, pros::E_MOTOR_GEAR_BLUE);
pros::Motor rightRearMotor(RIGHT_MOTOR_REAR, pros::E_MOTOR_GEAR_BLUE);
pros::MotorGroup leftMotors({leftFrontMotor, leftMidMotor, leftRearMotor});
pros::MotorGroup rightMotors({rightFrontMotor, rightMidMotor, rightRearMotor});

// ----------------------------------------------------------------------------
// Sensors
// ----------------------------------------------------------------------------
pros::Imu inertial(INERTIAL);
pros::Rotation leftRotation(LEFT_ROTATION, true);     // Reversed
pros::Rotation rearRotation(REAR_ROTATION, true);     // Reversed

// ============================================================================
// LEMLIB ODOMETRY & CHASSIS CONFIGURATION
// ============================================================================

// ----------------------------------------------------------------------------
// Tracking Wheels (for odometry - currently defined but not used)
// ----------------------------------------------------------------------------
// TODO: Once tracking wheel ports are fixed in globals.hpp, enable these in sensors
lemlib::TrackingWheel verticalTracking(&leftRotation, lemlib::Omniwheel::NEW_275, 0.0);
lemlib::TrackingWheel horizontalTracking(&rearRotation, lemlib::Omniwheel::NEW_2, 3.25);

// ----------------------------------------------------------------------------
// Drivetrain Configuration
// ----------------------------------------------------------------------------
lemlib::Drivetrain drivetrain {
    &leftMotors,                    // Left motor group
    &rightMotors,                   // Right motor group
    9.9,                            // Track width (inches) - distance between left/right wheels
    lemlib::Omniwheel::NEW_325,     // Wheel type: 3.25" omni wheels
    450,                            // Drivetrain RPM (with blue cartridge: 600 RPM * gear ratio)
    2                               // Horizontal drift correction (degrees)
};

// ----------------------------------------------------------------------------
// Odometry Sensors
// WARNING: Currently using IMU-only tracking (all tracking wheels set to nullptr)
// For better accuracy in competition:
//   1. Fix port conflicts in globals.hpp
//   2. Uncomment the tracking wheel parameters below
//   3. Test and tune tracking wheel offsets
// ----------------------------------------------------------------------------
lemlib::OdomSensors sensors(
    nullptr,        // Vertical tracking wheel 1 (left side) - TODO: use &verticalTracking
    nullptr,        // Vertical tracking wheel 2 (right side) - set to nullptr if not used
    nullptr,        // Horizontal tracking wheel 1 (rear) - TODO: use &horizontalTracking
    nullptr,        // Horizontal tracking wheel 2 - set to nullptr if not used
    &inertial       // IMU sensor (always required)
);

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

// ----------------------------------------------------------------------------
// Lateral PID (Forward/Backward Movement)
// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
// Angular PID (Turning)
// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
// Chassis Object (combines all drivetrain, PID, and sensors)
// ----------------------------------------------------------------------------
lemlib::Chassis chassis(drivetrain, lateralPID, angularPID, sensors);

// ============================================================================
// SUBSYSTEM INITIALIZATION
// ============================================================================

// ----------------------------------------------------------------------------
// Intake Subsystem
// ----------------------------------------------------------------------------
pros::Motor intakeMotor1(INTAKE_MOTOR_1, pros::E_MOTOR_GEARSET_18);  // 18:1 gearset (red)
pros::Motor intakeMotor2(INTAKE_MOTOR_2, pros::E_MOTOR_GEARSET_06);  // 6:1 gearset (blue)
pros::Motor intakeMotor3(INTAKE_MOTOR_3, pros::E_MOTOR_GEARSET_18);  // 18:1 gearset (red)
subsystems::Intake intake(
    {intakeMotor1, intakeMotor2, intakeMotor3},     // Motor group
    INTAKE_PISTON_PORT,                              // Pneumatic piston
    INTAKE_COLOR_SENSOR_PORT,                        // Optical sensor for color sorting
    INTAKE_LIMIT_PORT                                // Limit switch
);

// ----------------------------------------------------------------------------
// Arm Subsystem
// ----------------------------------------------------------------------------
pros::Motor rightArmMotor(RIGHT_ARM_MOTOR, pros::E_MOTOR_GEARSET_18);
pros::Motor leftArmMotor(LEFT_ARM_MOTOR, pros::E_MOTOR_GEARSET_18);
subsystems::Arm arm({leftArmMotor}, ARM_PISTON_PORT);

// ----------------------------------------------------------------------------
// Other Subsystems
// ----------------------------------------------------------------------------
subsystems::Clamp clamp(CLAMP_PORT);
subsystems::Doinker doinker(DOINKER_PORT);
subsystems::Movement movement(&chassis);
subsystems::Auton auton(&chassis);
subsystems::Selector selector(&intake, &auton);
subsystems::DistanceAlign distanceAlign(
    RIGHT_DISTANCE_PORT,    // Right distance sensor
    LEFT_DISTANCE_PORT,     // Left distance sensor
    6.5,                    // Sensor offset (inches)
    0.0                     // Angle offset (degrees)
);
