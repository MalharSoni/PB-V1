#pragma once

#include "pros/rtos.hpp"
#include "pros/misc.hpp"
#include "pros/motors.hpp"
#include "pros/adi.hpp"
#include "robot/movement.hpp"
#include "robot/auton.hpp"
#include "robot/intake_pushback.hpp"  // Push Back 3-stage intake
// #include "robot/intake.hpp"      // Old High Stakes (archived)
// #include "robot/arm.hpp"         // Old High Stakes (archived)
#include "lemlib/api.hpp"
// #include "robot/selector.hpp"    // Old High Stakes (archived - depends on old intake)
#include "robot/distance_align.hpp"  // High Stakes specific
#include "robot/telemetry.hpp"       // Telemetry logging system
#include "robot/alerts.hpp"          // Real-time alert system
#include "lib/Pneumatic.hpp"         // Generic pneumatic component library

// ============================================================================
// PORT CONFIGURATION
// ============================================================================
// All port conflicts have been resolved. Current port assignments:
//
// MOTORS (V5 Smart Ports):
//   Drivetrain: 12, 14, 15, 16 (4-motor drive - removed 11, 13)
//   Intake: 18, 19, 20
//
// SENSORS (V5 Smart Ports):
//   IMU: 10
//   Tracking wheels: 7, 8
//   Distance sensors: 3, 4
//
// PNEUMATICS (ADI Ports): C, H, B, D, F
// ============================================================================

// ----------------------------------------------------------------------------
// DRIVETRAIN MOTORS (V5 Smart Ports)
// ----------------------------------------------------------------------------
// 4-MOTOR DRIVE: 2 motors per side (removed 11 and 12 - no gearbox yet)
#define LEFT_MOTOR_FRONT -15    // Left front drive motor (reversed)
#define LEFT_MOTOR_MID -14      // Left middle drive motor (reversed)
// #define LEFT_MOTOR_REAR -11  // REMOVED - No gearbox yet
#define RIGHT_MOTOR_FRONT 16    // Right front drive motor
#define RIGHT_MOTOR_MID 13      // Right middle drive motor
// #define RIGHT_MOTOR_REAR 12  // REMOVED - No gearbox yet

// ----------------------------------------------------------------------------
// SUBSYSTEM MOTORS (V5 Smart Ports)
// ----------------------------------------------------------------------------
// Push Back Intake - 3 Stage System
#define INTAKE_STAGE_1 20       // Stage 1: 11W motor (main intake)
#define INTAKE_STAGE_2 19       // Stage 2: 5.5W motor (transfer/routing)
#define INTAKE_STAGE_3 18       // Stage 3: 5.5W motor (scoring)

// Old High Stakes motors (archived - can be reused for new season)
// #define RIGHT_ARM_MOTOR 22
// #define LEFT_ARM_MOTOR 22

// ----------------------------------------------------------------------------
// SENSORS (V5 Smart Ports)
// ----------------------------------------------------------------------------
#define INERTIAL 10                     // IMU sensor for heading/rotation
#define LEFT_ROTATION 8                 // Left tracking wheel (odometry)
#define REAR_ROTATION 7                 // Rear tracking wheel (odometry)
// #define INTAKE_COLOR_SENSOR_PORT 5   // Old High Stakes - Optical sensor (archived)
#define RIGHT_DISTANCE_PORT 3           // Right distance sensor (wall align)
#define LEFT_DISTANCE_PORT 4            // Left distance sensor (wall align)

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
// TELEMETRY CONFIGURATION
// ============================================================================
// Auto-start logging during autonomous (for PID tuning)
// Comment out this line to disable autonomous logging during competition
#define ENABLE_AUTON_LOGGING

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
    class BrainUI;
}

// ============================================================================
// GLOBAL HARDWARE & SUBSYSTEM OBJECTS
// ============================================================================

// ----------------------------------------------------------------------------
// Controller
// ----------------------------------------------------------------------------
extern pros::Controller master;

// ----------------------------------------------------------------------------
// Drivetrain Motors & Motor Groups (4-motor drive)
// ----------------------------------------------------------------------------
extern pros::Motor leftFrontMotor;
extern pros::Motor leftMidMotor;
// extern pros::Motor leftRearMotor;  // REMOVED - No gearbox yet
extern pros::Motor rightFrontMotor;
extern pros::Motor rightMidMotor;
// extern pros::Motor rightRearMotor;  // REMOVED - No gearbox yet
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
// Push Back intake motors
extern pros::Motor intakeStage1;
extern pros::Motor intakeStage2;
extern pros::Motor intakeStage3;

// Old High Stakes motors (archived)
// extern pros::Motor rightArmMotor;
// extern pros::Motor leftArmMotor;

// ----------------------------------------------------------------------------
// Subsystem Objects
// ----------------------------------------------------------------------------
extern subsystems::IntakePushback intake;  // Push Back 3-stage intake
// extern subsystems::Arm arm;  // Old High Stakes (archived)
extern subsystems::Movement movement;
// extern subsystems::Selector selector;  // Old High Stakes (archived)
extern subsystems::DistanceAlign distanceAlign;
extern subsystems::Auton auton;
extern subsystems::BrainUI brainUI;  // LVGL-based brain screen UI

// ----------------------------------------------------------------------------
// Reliability & Diagnostics Subsystems
// ----------------------------------------------------------------------------
extern subsystems::Telemetry telemetry;  // CSV logging to SD card
extern subsystems::Alerts alerts;        // Real-time controller alerts

// ----------------------------------------------------------------------------
// Generic Pneumatic Components (lib:: namespace)
// Students: Rename these based on your game!
// ----------------------------------------------------------------------------
extern lib::Pneumatic clamp;    // High Stakes: mobile goal clamp
extern lib::Pneumatic doinker;  // High Stakes: doinker mechanism
// Next season examples:
//   extern lib::Pneumatic wings;
//   extern lib::Pneumatic blocker;