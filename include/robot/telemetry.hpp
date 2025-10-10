#pragma once

#include "pros/rtos.hpp"
#include <cstdio>

namespace subsystems {

/**
 * @brief Telemetry logging system for autonomous analysis
 *
 * Logs robot state to CSV file on SD card for post-match analysis.
 * Captures position, velocity, motor health, and battery data.
 *
 * Usage:
 *   telemetry.init();           // Start logging
 *   telemetry.log();            // Call every 50ms
 *   telemetry.close();          // Stop logging
 */
class Telemetry {
public:
    Telemetry();

    /**
     * @brief Initialize telemetry logging
     * Creates new CSV file with headers
     * Call this at start of autonomous or driver control
     */
    void init();

    /**
     * @brief Log current robot state to CSV
     * Should be called periodically (every 50ms recommended)
     */
    void log();

    /**
     * @brief Close telemetry file
     * Call this at end of autonomous or driver control
     */
    void close();

    /**
     * @brief Check if telemetry is currently logging
     */
    bool isLogging() const { return logging; }

private:
    FILE* logfile;
    bool logging;
    uint32_t startTime;
    char currentFilename[64];  // Store filename for reopening
};

} // namespace subsystems
