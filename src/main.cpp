#include "main.h"
#include "globals.hpp"
#include "robot_config.hpp"
#include "robot/brain_ui.hpp"
#include "pros/misc.h"
#include "pros/motors.h"
#include "logging/tuning_logger.hpp"
#include "ui/runtime_controls.hpp"

using namespace subsystems;

// Forward declarations for background tasks
void tuning_telemetry_task(void* param);

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

    // ========================================================================
    // TELEMETRY SYSTEM - High-priority logging
    // ========================================================================
    // Initialize runtime controls (A/B/X/Y buttons for logging control)
    ui::runtime_controls_init();

    // Start high-priority telemetry task (100 Hz, captures data during motion)
    // NOTE: This task is ABOVE motion task priority to prevent starvation
    pros::Task tuning_telem_task(tuning_telemetry_task, nullptr,
                                 TASK_PRIORITY_DEFAULT + 1,  // High priority!
                                 TASK_STACK_DEPTH_DEFAULT,
                                 "Tuning Telemetry");

    // ========================================================================
    // GAME-SPECIFIC INITIALIZATION (Push Back)
    // ========================================================================
    // No game-specific initialization needed for Push Back intake
}

/**
 * Runs when robot is disabled (competition switch off).
 */
void disabled() {
    // Close telemetry logger to flush remaining data
    telem::tuning_logger_close();
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
    // TELEMETRY AUTO-LOGGING (For PID Tuning)
    // ========================================================================
    // Auto-start logging if ENABLE_AUTON_LOGGING is defined in globals.hpp
    // To disable for competition, comment out the #define in globals.hpp
    #ifdef ENABLE_AUTON_LOGGING
    pros::lcd::print(0, "Checking SD card...");
    pros::delay(100);

    if (pros::usd::is_installed()) {
        pros::lcd::print(0, "SD card OK!");
        pros::delay(200);

        // IMPORTANT: Close any existing logger first
        // This handles the case where manual logging was left active
        telem::tuning_logger_close();
        pros::delay(50);

        // Use descriptive filename: LemLib defaults
        if (telem::tuning_logger_init("lemlib_defaults")) {
            pros::lcd::print(1, "LOG: LemLib def");
            pros::lcd::print(0, "Logger init SUCCESS");
            printf("[AUTON] Auto-logging started\n");

            // Check if logger is actually ready
            if (telem::tuning_logger_ready()) {
                pros::lcd::print(0, "Logger READY!");
            } else {
                pros::lcd::print(0, "Logger NOT READY!");
            }

            // Give writer task time to start and flush header to SD card
            pros::delay(200);
        } else {
            pros::lcd::print(1, "LOG FAILED!");
            pros::lcd::print(0, "Init returned FALSE");
            printf("[AUTON] Failed to start logger\n");
            pros::delay(2000);  // Show error
        }
    } else {
        pros::lcd::print(1, "NO SD CARD!");
        pros::lcd::print(0, "usd::is_installed() = false");
        printf("[AUTON] No SD card detected\n");
        pros::delay(2000);  // Show error
    }
    #endif

    // ========================================================================
    // PID TUNING TESTS - ISOLATED (includes telemetry logging)
    // ========================================================================

    // SIMPLE TEST: Just drive forward 48 inches (no turn)
    // Speed: 100 (full speed for fast autonomous)
    chassis.setPose(0, 0, 0);
    pros::lcd::set_text(2, "Test: 48\" @ SPEED 100");
    pros::delay(1000);

    pros::lcd::set_text(3, "LemLib Defaults...");
    chassis.moveToPoint(0, 48, 5000, {
        .forwards = true,
        .maxSpeed = 100
        // Using LemLib recommended defaults: kP=10, kD=3, slew=20
    });
    chassis.waitUntilDone();
    pros::delay(1000);

    lemlib::Pose final = chassis.getPose();
    pros::lcd::set_text(3, "TEST COMPLETE");
    pros::lcd::print(4, "Y: %.1f\" (target 48)", final.y);
    pros::delay(3000);

    // ========================================================================
    // OTHER TESTS
    // ========================================================================

    // MOTOR DIAGNOSTICS - Check for motor imbalance
    // auton.motorDiagnostics();

    // COMPLETE ODOMETRY TEST - Out and back with turns
    // auton.odomDriveTest();

    // ODOMETRY TUNING TEST - Drive in a square
    // auton.odomSquareTest();

    // ========================================================================
    // GAME ROUTINES
    // ========================================================================

    // Run selected autonomous routine from UI
    // auton.run_auton(brainUI.getSelectedAuton());

    // Push Back game routines:
    // auton.pushBackSimple();  // Simple intake and score routine

    // Old High Stakes routines (archived):
    // auton.elimSAFERED();
    // auton.elimSAFEBLUE();
    // auton.swpRED();
    // auton.swpBLUE();
    // auton.skills();

    // ========================================================================
    // TELEMETRY AUTO-CLOSE
    // ========================================================================
    #ifdef ENABLE_AUTON_LOGGING
    // Get stats before closing
    slog::Stats stats;
    telem::tuning_logger_get_stats(stats);

    pros::lcd::print(0, "Closing logger...");
    pros::lcd::print(2, "Lines: %u Drops: %u", stats.lines, stats.drops);
    pros::lcd::print(3, "Q: %d/%d Hi: %d", stats.queue_depth, stats.queue_capacity, stats.high_water);

    // Give writer task time to flush remaining data to SD card
    // Increased delay to ensure all buffered data is written
    pros::delay(500);

    telem::tuning_logger_close();

    pros::lcd::print(1, "LOG CLOSED");
    pros::lcd::print(0, "Check SD card!");
    printf("[AUTON] Auto-logging stopped\n");

    // Hold stats on screen for 3 seconds
    pros::delay(3000);
    #endif
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
 * @brief High-priority telemetry logging task (100 Hz)
 * Runs at TASK_PRIORITY_DEFAULT + 1 to capture data during motion
 */
void tuning_telemetry_task(void* param) {
    while (true) {
        telem::tuning_logger_tick();
        pros::delay(10);  // 100 Hz
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

    // Runtime controls update counter
    int ui_counter = 0;

    // IMU health check counter (check every 2 seconds)
    int imu_check_counter = 0;

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
        // NOTE: Y button now controls telemetry (rotate file)
        clamp.run(DIGITAL_DOWN);   // Down button: clamp
        doinker.run(DIGITAL_RIGHT); // Right button: doinker

        // ====================================================================
        // TELEMETRY RUNTIME CONTROLS (A/B/X/Y buttons)
        // ====================================================================
        // A: Toggle logging on/off
        // B: Cycle rate (100/50/25/10 Hz)
        // X: Set waypoint marker
        // Y: Rotate log file
        // (Wall alignment moved to D-pad)
        // ====================================================================

        // Update runtime controls every 100ms
        ui_counter++;
        if (ui_counter >= 10) {  // 10 * 10ms = 100ms
            ui::runtime_controls_update();
            ui_counter = 0;
        }

        // ====================================================================
        // IMU HEALTH MONITORING
        // ====================================================================
        // Check IMU status every 2 seconds
        imu_check_counter++;
        if (imu_check_counter >= 200) {  // 200 * 10ms = 2 seconds
            if (!robot_config::check_imu_status()) {
                master.print(2, 0, "IMU ERROR!");
                printf("[OPCONTROL] IMU error detected!\n");
            }
            imu_check_counter = 0;
        }

        // ====================================================================
        // WALL ALIGNMENT (Optional - moved to D-pad)
        // ====================================================================

        // Down+Left: Align angle to wall
        if (master.get_digital_new_press(DIGITAL_LEFT)) {
            distanceAlign.calculateAngleOneWall(0.0);
            alerts.resetDriftTimer();
        }

        // Down+Up: Align distance to wall
        if (master.get_digital_new_press(DIGITAL_UP)) {
            distanceAlign.calculateDistOneWall(0.0, 72.0);
            alerts.resetDriftTimer();
        }

        pros::delay(10);  // Small delay to prevent CPU overload
    }

    // ========================================================================
    // CLEANUP - Stop telemetry logging
    // ========================================================================
    telemetry.close();
}
