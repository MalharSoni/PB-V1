#include "main.h"
#include "globals.hpp"
#include "robot_config.hpp"
#include "pros/misc.h"
#include "pros/motors.h"

using namespace subsystems;

/**
 * Runs on robot power-on.
 * Used for sensor calibration and initial configuration.
 */
void initialize() {
    pros::lcd::initialize();

    // Initialize all robot hardware (IMU, chassis, subsystems, etc.)
    robot_config::initialize();

    // Set initial robot pose (starting position on field)
    chassis.setPose(0, 0, 0);

    // Set drivetrain brake modes
    leftMotors.set_brake_modes(pros::motor_brake_mode_e::E_MOTOR_BRAKE_BRAKE);
    rightMotors.set_brake_modes(pros::motor_brake_mode_e::E_MOTOR_BRAKE_BRAKE);

    // ========================================================================
    // GAME-SPECIFIC INITIALIZATION (Push Back)
    // ========================================================================
    // No game-specific initialization needed for Push Back intake
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
    // Old High Stakes selector (archived)
    // selector.init();
    // while (true) {
    //     selector.update();
    //     pros::delay(200);
    // }

    // For now, just show robot is ready
    pros::lcd::print(0, "Push Back Robot Ready!");
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

        // Push Back Intake - 4 Button Control
        intake.run(DIGITAL_R1,   // R1: Intake from floor
                   DIGITAL_R2,   // R2: Score Level 1 / Outtake
                   DIGITAL_L1,   // L1: Score Level 2
                   DIGITAL_L2);  // L2: Score Level 3

        // Pneumatics (if still using clamp/doinker from High Stakes)
        clamp.run(DIGITAL_Y);      // Y button: clamp
        doinker.run(DIGITAL_RIGHT); // Right button: doinker

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
