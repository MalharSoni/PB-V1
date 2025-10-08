#include "main.h"
#include "globals.hpp"
#include "robot_config.hpp"
#include "pros/misc.h"
#include "pros/motors.h"

using namespace subsystems;

/**
 * @brief Background task to update LCD with robot position
 * Displays X, Y, and heading for easy autonomous route coding
 * Also shows raw sensor values for tracking wheel diagnosis
 */
void lcd_position_task(void* param) {
    while (true) {
        lemlib::Pose pose = chassis.getPose();

        // Update line 5 with raw sensor values for diagnosis
        int leftSensor = leftRotation.get_position();
        int rearSensor = rearRotation.get_position();
        pros::lcd::print(5, "L:%d R:%d", leftSensor, rearSensor);

        // Update line 6 with position data (using C API for printf formatting)
        pros::lcd::print(6, "X:%.2f Y:%.2f H:%.1f",
                        pose.x, pose.y, pose.theta);

        pros::delay(100);  // Update every 100ms
    }
}

/**
 * Runs on robot power-on.
 * Used for sensor calibration and initial configuration.
 */
void initialize() {
    // Initialize PROS LCD (simple text display)
    pros::lcd::initialize();

    // Set black background with white text
    pros::lcd::set_background_color(0, 0, 0);      // Black (R=0, G=0, B=0)
    pros::lcd::set_text_color(255, 255, 255);      // White (R=255, G=255, B=255)

    // Display team info on brain screen
    pros::lcd::set_text(0, "=============================");
    pros::lcd::set_text(1, "");
    pros::lcd::set_text(2, "   /// CAUTION TAPE ///");
    pros::lcd::set_text(3, "   /// TEAM 839Y ///");
    pros::lcd::set_text(4, "");
    pros::lcd::set_text(5, "L:0 R:0");  // Raw sensor values (updated by task)
    pros::lcd::set_text(6, "X:0.00 Y:0.00 H:0.0");  // Position (updated by task)
    pros::lcd::set_text(7, "=============================");

    // Initialize all robot hardware (IMU, chassis, subsystems, etc.)
    robot_config::initialize();

    // Set initial robot pose (starting position on field)
    chassis.setPose(0, 0, 0);

    // Set drivetrain brake modes
    leftMotors.set_brake_modes(pros::motor_brake_mode_e::E_MOTOR_BRAKE_BRAKE);
    rightMotors.set_brake_modes(pros::motor_brake_mode_e::E_MOTOR_BRAKE_BRAKE);

    // Start background task to display position data
    pros::Task lcd_task(lcd_position_task, nullptr, "LCD Position");

    // ========================================================================
    // RELIABILITY SYSTEMS
    // ========================================================================
    // Initialize alert system odometry drift timer
    alerts.resetDriftTimer();

    // Characterize IMU drift (robot MUST be still!)
    pros::lcd::set_text(3, "Measuring IMU drift...");
    if (imuDrift.characterize()) {
        pros::lcd::print(3, "IMU: %.4f deg/s drift", imuDrift.getDriftRate());
    } else {
        pros::lcd::set_text(3, "IMU: MOVED - No drift comp");
    }
    pros::delay(1000);

    // ========================================================================
    // SIMULATION MODE
    // ========================================================================
    // STUDENTS: Uncomment to enable simulation mode (test without robot!)
    simulation.setEnabled(true);  // Enable simulation
    // simulation.reset();            // Reset to starting position
}

/**
 * Runs when robot is disabled (competition switch off).
 */
void disabled() {}

/**
 * @brief Background task for simulation updates
 */
void simulation_task(void* param) {
    while (true) {
        if (simulation.isEnabled()) {
            simulation.update();
            // Optional: Print state every second
            // static uint32_t lastPrint = 0;
            // if (pros::millis() - lastPrint > 1000) {
            //     simulation.printState();
            //     lastPrint = pros::millis();
            // }
        }
        pros::delay(10);  // Update every 10ms
    }
}

/**
 * Runs when competition switch is connected.
 * Used for autonomous routine selection.
 */
void competition_initialize() {
    // Display competition ready message
    pros::lcd::clear();

    // Ensure black background with white text
    pros::lcd::set_background_color(0, 0, 0);      // Black
    pros::lcd::set_text_color(255, 255, 255);      // White

    pros::lcd::set_text(0, "=============================");
    pros::lcd::set_text(1, "");
    pros::lcd::set_text(2, "    COMPETITION MODE");
    pros::lcd::set_text(3, "");
    pros::lcd::set_text(4, "   /// TEAM 839Y ///");
    pros::lcd::set_text(5, "");
    pros::lcd::set_text(6, "      READY!");
    pros::lcd::set_text(7, "=============================");

    // Old High Stakes selector (archived)
    // selector.init();
    // while (true) {
    //     selector.update();
    //     pros::delay(200);
    // }
}

/**
 * 15-second autonomous period.
 * STUDENTS: Uncomment the autonomous routine you want to run.
 */
void autonomous() {
    // ========================================================================
    // SIMULATION MODE - Start simulation background task
    // ========================================================================
    pros::Task simulation_bg(simulation_task, nullptr, "Simulation");

    // ========================================================================
    // TELEMETRY LOGGING - Start recording autonomous
    // ========================================================================
    telemetry.init();
    alerts.resetDriftTimer();

    // Start IMU drift compensation
    imuDrift.startCompensation();

    // Run selected autonomous routine from UI
    // auton.run_auton(brainUI.getSelectedAuton());

    // MOTOR DIAGNOSTICS - Check for motor imbalance
    // auton.motorDiagnostics();

    // COMPLETE ODOMETRY TEST - Out and back with turns
    auton.odomDriveTest();

    // ODOMETRY TUNING TEST - Drive in a square
    // auton.odomSquareTest();

    // Push Back game routines:
    // auton.pushBackSimple();  // Simple intake and score routine

    // Old High Stakes routines (archived):
    // auton.elimSAFERED();
    // auton.elimSAFEBLUE();
    // auton.swpRED();
    // auton.swpBLUE();
    // auton.skills();

    // ========================================================================
    // SIMULATION MODE - Show final state
    // ========================================================================
    if (simulation.isEnabled()) {
        simulation.printState();
        simulation.drawField();
    }

    // ========================================================================
    // TELEMETRY LOGGING - Stop recording
    // ========================================================================
    telemetry.close();
}

/**
 * @brief Background task for telemetry logging during driver control
 */
void telemetry_task(void* param) {
    while (true) {
        if (telemetry.isLogging()) {
            telemetry.log();
        }
        pros::delay(50);  // Log every 50ms
    }
}

/**
 * @brief Background task for real-time alerts
 */
void alerts_task(void* param) {
    while (true) {
        alerts.check();
        pros::delay(100);  // Check every 100ms
    }
}

/**
 * Driver control period (unlimited time).
 * STUDENTS: This is where you map controller buttons to robot actions.
 */
void opcontrol() {
    // Set brake modes for driver control
    leftMotors.set_brake_modes(pros::motor_brake_mode_e::E_MOTOR_BRAKE_BRAKE);
    rightMotors.set_brake_modes(pros::motor_brake_mode_e::E_MOTOR_BRAKE_BRAKE);

    // ========================================================================
    // RELIABILITY SYSTEMS - Start monitoring
    // ========================================================================
    telemetry.init();
    alerts.resetDriftTimer();

    // Start background tasks for telemetry and alerts
    pros::Task telemetry_bg(telemetry_task, nullptr, "Telemetry");
    pros::Task alerts_bg(alerts_task, nullptr, "Alerts");

    // ========================================================================
    // LVGL UI - DISABLED (causes crash)
    // ========================================================================
    // Using simple PROS LCD with black background instead
    // brainUI.init();
    // brainUI.showSplash();
    // pros::delay(2000);
    // brainUI.showOperationScreen();

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
            alerts.resetDriftTimer();  // Reset drift timer after correction
        }

        // X button: Align distance to wall
        if (master.get_digital_new_press(DIGITAL_X)) {
            distanceAlign.calculateDistOneWall(0.0, 72.0);
            alerts.resetDriftTimer();  // Reset drift timer after correction
        }

        // ====================================================================
        // UI UPDATES
        // ====================================================================

        // LVGL disabled - using simple LCD display instead
        // brainUI.updateTelemetry();

        // Controller screen display (Note: Alerts will override this periodically)
        // master.print(0, 0, "x:%5.3Lf y:%5.3Lf", chassis.getPose().x, chassis.getPose().y);

        pros::delay(10);  // Small delay to prevent CPU overload
    }

    // ========================================================================
    // CLEANUP - Stop telemetry logging
    // ========================================================================
    telemetry.close();
}
