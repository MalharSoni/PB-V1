#pragma once

/**
 * @file tuning_logger.hpp
 * @brief High-level orchestration for telemetry logging
 *
 * Combines:
 * - telemetry_adapter (data gathering)
 * - telemetry_stream (CSV formatting)
 * - slog (ring buffer + writer)
 *
 * Usage:
 *   tuning_logger_init("lateral_pid");
 *   // In 100 Hz task:
 *   tuning_logger_tick();
 *   // At end:
 *   tuning_logger_close();
 */

#include "slog.hpp"
#include "telemetry_adapter.hpp"
#include "telemetry_stream.hpp"
#include "pros/apix.h"

namespace telem {

// ============================================================================
// RATE CONTROL
// ============================================================================

/**
 * @brief Get/set logging divisor (rate throttle)
 * @return Reference to divisor (1=100Hz, 2=50Hz, 4=25Hz, 10=10Hz)
 *
 * Default: 1 (log every tick)
 */
inline int& log_div() {
    static int divisor = 1;
    return divisor;
}

/**
 * @brief Set logging rate
 * @param hz Target frequency (100, 50, 25, or 10 Hz)
 */
inline void set_log_rate(int hz) {
    if (hz == 100) log_div() = 1;
    else if (hz == 50) log_div() = 2;
    else if (hz == 25) log_div() = 4;
    else if (hz == 10) log_div() = 10;
    else log_div() = 1;  // Default to 100 Hz
}

/**
 * @brief Get current logging rate in Hz
 * @return Logging frequency
 */
inline int get_log_rate() {
    return 100 / log_div();
}

// ============================================================================
// LIFECYCLE
// ============================================================================

/**
 * @brief Initialize tuning logger
 * @param hint Filename hint (e.g., "lateral_pid", "angular_pid", "auton")
 * @return true if initialization succeeded
 */
inline bool tuning_logger_init(const char* hint = "tuning") {
    return slog::init(hint, kHeader);
}

/**
 * @brief Check if logger is ready
 * @return true if ready to log
 */
inline bool tuning_logger_ready() {
    return slog::ready();
}

/**
 * @brief Log one telemetry sample (call at 100 Hz)
 *
 * Gathers data from sensors, formats CSV row, and enqueues to ring buffer.
 * Respects log_div() throttling.
 */
inline void tuning_logger_tick() {
    if (!slog::ready()) {
        return;
    }

    // Gather data
    const int t = pros::millis();
    auto p = get_pose();
    double vl = 0, vr = 0;
    get_wheel_vel(vl, vr);
    double batt = get_batt_V();
    const char* mark = get_mark();

    // Format CSV row
    char line[256];
    format_row(line, sizeof(line), t, p.x, p.y, p.theta, vl, vr, batt, mark);

    // Enqueue (throttled)
    slog::throttled_logf(log_div(), "%s", line);
}

/**
 * @brief Close tuning logger and flush data
 */
inline void tuning_logger_close() {
    slog::close();
}

/**
 * @brief Get logger statistics
 * @param out Stats struct to fill
 */
inline void tuning_logger_get_stats(slog::Stats& out) {
    slog::get_stats(out);
}

} // namespace telem
