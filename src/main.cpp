#include "main.h"
#include "globals.hpp"
#include "pros/misc.h"
#include "pros/motors.h"

using namespace subsystems;

/**
 * Runs on robot power-on.
 * Used for sensor calibration and initial configuration.
 */
void initialize() {
    pros::lcd::initialize();

    // Calibrate IMU (IMPORTANT: Robot must be still during calibration!)
    chassis.calibrate();
    chassis.setPose(0, 0, 0);

    // Set drivetrain brake modes
    leftMotors.set_brake_modes(pros::motor_brake_mode_e::E_MOTOR_BRAKE_BRAKE);
    rightMotors.set_brake_modes(pros::motor_brake_mode_e::E_MOTOR_BRAKE_BRAKE);

    // Set default alliance color for intake sorting
    intake.set_target_color(DONUT_COLOR::RED);
}

/**
 * Runs when robot is disabled (competition switch off).
 */
void disabled() {}

/**
 * Runs when competition switch is connected.
 * Used for autonomous routine selection.
 */
void competition_initialize() {
    selector.init();
    while (true) {
        selector.update();
        pros::delay(200);
    }
}

/**
 * 15-second autonomous period.
 * STUDENTS: Uncomment the autonomous routine you want to run.
 */
void autonomous() {
    // Uncomment ONE of these routines:
    // auton.elimSAFERED();
    // auton.elimSAFEBLUE();
    // auton.swpRED();
    // auton.swpBLUE();
    // auton.skills();
}

/**
 * Driver control period (unlimited time).
 * STUDENTS: This is where you map controller buttons to robot actions.
 */
void opcontrol() {
    // Set brake modes for driver control
    leftMotors.set_brake_modes(pros::motor_brake_mode_e::E_MOTOR_BRAKE_BRAKE);
    rightMotors.set_brake_modes(pros::motor_brake_mode_e::E_MOTOR_BRAKE_BRAKE);

    while (true) {
        // ====================================================================
        // DRIVER CONTROLS
        // ====================================================================

        // Drivetrain: Arcade drive (left stick Y + right stick X)
        movement.arcade_drive(master.get_analog(ANALOG_LEFT_Y),
                             master.get_analog(ANALOG_RIGHT_X));

        // Subsystem controls
        clamp.run(DIGITAL_Y);                                           // Y button: clamp
        intake.run(DIGITAL_R1, DIGITAL_L1, DIGITAL_UP, DIGITAL_LEFT);  // R1/L1: intake, Up/Left: other
        arm.run(DIGITAL_DOWN, DIGITAL_B, DIGITAL_UP, DIGITAL_L2, DIGITAL_R2);  // Arm control
        doinker.run(DIGITAL_RIGHT);                                     // Right button: doinker

        // ====================================================================
        // WALL ALIGNMENT (Optional - for precision positioning)
        // ====================================================================

        // A button: Align angle to wall
        if (master.get_digital_new_press(DIGITAL_A)) {
            distanceAlign.calculateAngleOneWall(0.0);
        }

        // X button: Align distance to wall
        if (master.get_digital_new_press(DIGITAL_X)) {
            distanceAlign.calculateDistOneWall(0.0, 72.0);
        }

        // ====================================================================
        // DEBUG OUTPUT
        // ====================================================================

        // Brain LCD display (rows 1-3)
        pros::lcd::print(1, "x: %Lf", chassis.getPose().x);
        pros::lcd::print(2, "y: %Lf", chassis.getPose().y);
        pros::lcd::print(3, "theta: %Lf", chassis.getPose().theta);

        // Controller screen display
        master.print(0, 0, "x:%5.3Lf y:%5.3Lf", chassis.getPose().x, chassis.getPose().y);

        pros::delay(10);  // Small delay to prevent CPU overload
    }
}
