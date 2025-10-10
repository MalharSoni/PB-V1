#pragma once

/**
 * @file runtime_controls.hpp
 * @brief Runtime controls for telemetry logger
 *
 * Controller buttons:
 * - A: Toggle logging on/off (close/reopen file)
 * - B: Cycle logging rate (100 → 50 → 25 → 10 → 100 Hz)
 * - X: Set waypoint marker (MARK:wp=N)
 * - Y: Rotate log file (close current, open new)
 *
 * LCD display (lines 1-3):
 * - L1: "LOG ON 50Hz" or "LOG OFF"
 * - L2: "Q: 45/512 Hi: 128"
 * - L3: "Drops: 0 Lines: 1234"
 *
 * Call runtime_controls_init() in initialize()
 * Call runtime_controls_update() in opcontrol() loop
 */

#include "logging/tuning_logger.hpp"
#include "logging/telemetry_adapter.hpp"
#include "pros/llemu.hpp"
#include "globals.hpp"

namespace ui {

// ============================================================================
// STATE
// ============================================================================

static bool logging_active = false;
static int rotation_counter = 0;

// ============================================================================
// BUTTON HANDLERS
// ============================================================================

/**
 * @brief Toggle logging on/off (A button)
 */
inline void handle_toggle_logging() {
    if (logging_active) {
        // Turn off
        telem::tuning_logger_close();
        logging_active = false;
        pros::lcd::print(1, "LOG OFF");
    } else {
        // Check if SD card is inserted before trying to log
        if (!pros::usd::is_installed()) {
            pros::lcd::print(1, "NO SD CARD!");
            master.rumble("-");  // Error rumble pattern
            return;
        }

        // Turn on
        char hint[32];
        snprintf(hint, sizeof(hint), "run_%d", rotation_counter);
        if (telem::tuning_logger_init(hint)) {
            logging_active = true;
            pros::lcd::print(1, "LOG ON %dHz", telem::get_log_rate());
        } else {
            pros::lcd::print(1, "LOG FAILED!");
            master.rumble("-");  // Error rumble pattern
        }
    }
}

/**
 * @brief Cycle logging rate (B button)
 * 100 Hz → 50 Hz → 25 Hz → 10 Hz → 100 Hz
 */
inline void handle_cycle_rate() {
    if (!logging_active) {
        return;  // Only works when logging
    }

    int current_rate = telem::get_log_rate();
    int new_rate;

    if (current_rate == 100) new_rate = 50;
    else if (current_rate == 50) new_rate = 25;
    else if (current_rate == 25) new_rate = 10;
    else new_rate = 100;

    telem::set_log_rate(new_rate);
    pros::lcd::print(1, "LOG ON %dHz", new_rate);
}

/**
 * @brief Set waypoint marker (X button)
 */
inline void handle_set_marker() {
    if (!logging_active) {
        return;  // Only works when logging
    }

    telem::set_waypoint_marker();
    master.rumble(".");  // Quick rumble confirmation
}

/**
 * @brief Rotate log file (Y button)
 * Closes current file and opens a new one
 */
inline void handle_rotate_file() {
    if (!logging_active) {
        return;  // Only works when logging
    }

    // Check SD card is still present
    if (!pros::usd::is_installed()) {
        logging_active = false;
        pros::lcd::print(1, "SD CARD REMOVED!");
        master.rumble("-");
        return;
    }

    // Close current
    telem::tuning_logger_close();

    // Open new with incremented counter
    rotation_counter++;
    char hint[32];
    snprintf(hint, sizeof(hint), "run_%d", rotation_counter);

    if (telem::tuning_logger_init(hint)) {
        pros::lcd::print(1, "LOG ROTATED %d", rotation_counter);
    } else {
        logging_active = false;
        pros::lcd::print(1, "ROTATE FAILED!");
        master.rumble("-");
    }
}

// ============================================================================
// LCD UPDATE
// ============================================================================

/**
 * @brief Update LCD with current stats
 */
inline void update_lcd() {
    if (!logging_active) {
        pros::lcd::print(1, "LOG OFF");
        pros::lcd::print(2, "");
        pros::lcd::print(3, "");
        return;
    }

    // Get stats
    slog::Stats stats;
    telem::tuning_logger_get_stats(stats);

    // Line 1: Status + rate
    pros::lcd::print(1, "LOG ON %dHz", telem::get_log_rate());

    // Line 2: Queue depth
    pros::lcd::print(2, "Q: %d/%d Hi: %d",
                     stats.queue_depth, stats.queue_capacity, stats.high_water);

    // Line 3: Drops + lines
    pros::lcd::print(3, "Drops: %u Lines: %u", stats.drops, stats.lines);
}

// ============================================================================
// MAIN UPDATE FUNCTION
// ============================================================================

/**
 * @brief Process button inputs and update LCD
 * Call this every ~100ms in opcontrol() loop
 */
inline void runtime_controls_update() {
    // Check buttons (new press only)
    if (master.get_digital_new_press(DIGITAL_A)) {
        handle_toggle_logging();
    }
    if (master.get_digital_new_press(DIGITAL_B)) {
        handle_cycle_rate();
    }
    if (master.get_digital_new_press(DIGITAL_X)) {
        handle_set_marker();
    }
    if (master.get_digital_new_press(DIGITAL_Y)) {
        handle_rotate_file();
    }

    // Update LCD
    update_lcd();
}

/**
 * @brief Initialize runtime controls
 * Call once in initialize()
 */
inline void runtime_controls_init() {
    logging_active = false;
    rotation_counter = 0;
    pros::lcd::print(1, "LOG OFF");
    pros::lcd::print(2, "A=Toggle B=Rate");
    pros::lcd::print(3, "X=Mark Y=Rotate");
}

} // namespace ui
