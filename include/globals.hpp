#pragma once

#include "pros/rtos.hpp"
#include "pros/misc.hpp"
#include "pros/motors.hpp"
#include "pros/adi.hpp"
#include "robot/movement.hpp"
#include "robot/auton.hpp"
#include "robot/clamp.hpp"
#include "robot/intake.hpp"
#include "robot/arm.hpp"
#include "robot/doinker.hpp"
#include "lemlib/api.hpp"
#include "robot/selector.hpp"
#include "robot/distance_align.hpp"

// ============================================================================
// PORT CONFIGURATION
// ============================================================================

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// TODO: FIX THESE PORT CONFLICTS BEFORE COMPETITION!
// The following devices all use port 22 and need unique ports assigned:
//   - LEFT_ROTATION (odometry tracking wheel)
//   - REAR_ROTATION (odometry tracking wheel)
//   - RIGHT_ARM_MOTOR
//   - LEFT_ARM_MOTOR
//   - INTAKE_COLOR_SENSOR_PORT (optical sensor)
//   - RIGHT_DISTANCE_PORT
//   - LEFT_DISTANCE_PORT
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// ----------------------------------------------------------------------------
// DRIVETRAIN MOTORS (V5 Smart Ports)
// ----------------------------------------------------------------------------
#define LEFT_MOTOR_FRONT -15    // Left front drive motor (reversed)
#define LEFT_MOTOR_MID -14      // Left middle drive motor (reversed)
#define LEFT_MOTOR_REAR -11     // Left rear drive motor (reversed)
#define RIGHT_MOTOR_FRONT 16    // Right front drive motor
#define RIGHT_MOTOR_MID 12      // Right middle drive motor
#define RIGHT_MOTOR_REAR 13     // Right rear drive motor

// ----------------------------------------------------------------------------
// SUBSYSTEM MOTORS (V5 Smart Ports)
// ----------------------------------------------------------------------------
#define INTAKE_MOTOR_1 20       // Intake motor 1 (18:1 gearset)
#define INTAKE_MOTOR_2 18       // Intake motor 2 (6:1 gearset)
#define INTAKE_MOTOR_3 19       // Intake motor 3 (18:1 gearset)
#define RIGHT_ARM_MOTOR 22      // TODO: CHANGE - conflicts with other devices!
#define LEFT_ARM_MOTOR 22       // TODO: CHANGE - conflicts with other devices!

// ----------------------------------------------------------------------------
// SENSORS (V5 Smart Ports)
// ----------------------------------------------------------------------------
#define INERTIAL 10                     // IMU sensor for heading/rotation
#define LEFT_ROTATION 22                // TODO: CHANGE - Left tracking wheel (odometry)
#define REAR_ROTATION 22                // TODO: CHANGE - Rear tracking wheel (odometry)
#define INTAKE_COLOR_SENSOR_PORT 22     // TODO: CHANGE - Optical sensor for ring sorting
#define RIGHT_DISTANCE_PORT 22          // TODO: CHANGE - Distance sensor (wall align)
#define LEFT_DISTANCE_PORT 22           // TODO: CHANGE - Distance sensor (wall align)

// ----------------------------------------------------------------------------
// PNEUMATICS (ADI Ports A-H)
// ----------------------------------------------------------------------------
#define CLAMP_PORT 'C'          // Mobile goal clamp solenoid
#define DOINKER_PORT 'H'        // Doinker mechanism solenoid
#define INTAKE_PISTON_PORT 'B'  // Intake raise/lower solenoid
#define ARM_PISTON_PORT 'D'     // Arm mechanism solenoid
#define INTAKE_LIMIT_PORT 'F'   // Intake limit switch (digital input)

// ============================================================================
// CONTROLLER BUTTON MAPPINGS
// ============================================================================
#define DIGITAL_L1 pros::E_CONTROLLER_DIGITAL_L1
#define DIGITAL_L2 pros::E_CONTROLLER_DIGITAL_L2
#define DIGITAL_R1 pros::E_CONTROLLER_DIGITAL_R1
#define DIGITAL_R2 pros::E_CONTROLLER_DIGITAL_R2
#define DIGITAL_A pros::E_CONTROLLER_DIGITAL_A
#define DIGITAL_B pros::E_CONTROLLER_DIGITAL_B
#define DIGITAL_X pros::E_CONTROLLER_DIGITAL_X
#define DIGITAL_Y pros::E_CONTROLLER_DIGITAL_Y
#define DIGITAL_RIGHT pros::E_CONTROLLER_DIGITAL_RIGHT
#define DIGITAL_LEFT pros::E_CONTROLLER_DIGITAL_LEFT
#define DIGITAL_UP pros::E_CONTROLLER_DIGITAL_UP
#define DIGITAL_DOWN pros::E_CONTROLLER_DIGITAL_DOWN
#define ANALOG_LEFT_Y pros::E_CONTROLLER_ANALOG_LEFT_Y
#define ANALOG_RIGHT_Y pros::E_CONTROLLER_ANALOG_RIGHT_Y
#define ANALOG_RIGHT_X pros::E_CONTROLLER_ANALOG_RIGHT_X

// ============================================================================
// GLOBAL TOGGLE VARIABLES
// ============================================================================
extern bool pistonToggle;
extern bool intakeToggle;
extern bool wallToggle;

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================
namespace subsystems {
    class Movement;
    class Selector;
    class Auton;
}

// ============================================================================
// GLOBAL HARDWARE & SUBSYSTEM OBJECTS
// ============================================================================

// ----------------------------------------------------------------------------
// Controller
// ----------------------------------------------------------------------------
extern pros::Controller master;

// ----------------------------------------------------------------------------
// Drivetrain Motors & Motor Groups
// ----------------------------------------------------------------------------
extern pros::Motor leftFrontMotor;
extern pros::Motor leftMidMotor;
extern pros::Motor leftRearMotor;
extern pros::Motor rightFrontMotor;
extern pros::Motor rightMidMotor;
extern pros::Motor rightRearMotor;
extern pros::MotorGroup leftMotors;
extern pros::MotorGroup rightMotors;

// ----------------------------------------------------------------------------
// Sensors
// ----------------------------------------------------------------------------
extern pros::Imu inertial;
extern pros::Rotation leftRotation;
extern pros::Rotation rearRotation;

// ----------------------------------------------------------------------------
// LemLib Chassis Configuration
// ----------------------------------------------------------------------------
extern lemlib::Drivetrain drivetrain;
extern lemlib::TrackingWheel verticalTracking;
extern lemlib::TrackingWheel horizontalTracking;
extern lemlib::OdomSensors sensors;
extern lemlib::ControllerSettings lateralPID;
extern lemlib::ControllerSettings angularPID;
extern lemlib::Chassis chassis;

// ----------------------------------------------------------------------------
// Subsystem Motors
// ----------------------------------------------------------------------------
extern pros::Motor intakeMotor1;
extern pros::Motor rightArmMotor;
extern pros::Motor leftArmMotor;

// ----------------------------------------------------------------------------
// Subsystem Objects
// ----------------------------------------------------------------------------
extern subsystems::Intake intake;
extern subsystems::Arm arm;
extern subsystems::Clamp clamp;
extern subsystems::Doinker doinker;
extern subsystems::Movement movement;
extern subsystems::Selector selector;
extern subsystems::DistanceAlign distanceAlign;
extern subsystems::Auton auton;