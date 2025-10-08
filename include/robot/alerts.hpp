#pragma once

#include "pros/misc.hpp"
#include <string>

namespace subsystems {

/**
 * @brief Real-time alert system for driver awareness
 *
 * Monitors robot health and displays critical warnings on controller.
 * Alerts driver to motor overheating, disconnections, battery issues, etc.
 *
 * Usage:
 *   alerts.check();  // Call every 100ms in opcontrol loop
 */
class Alerts {
public:
    enum Priority {
        INFO,       // Informational (no rumble)
        WARNING,    // Warning (single rumble pulse)
        CRITICAL    // Critical (triple rumble pulse)
    };

    Alerts();

    /**
     * @brief Check all robot systems and display alerts
     * Call this periodically (every 100ms) during driver control
     */
    void check();

    /**
     * @brief Reset odometry drift timer
     * Call this after wall alignment or GPS corrections
     */
    void resetDriftTimer();

    /**
     * @brief Get time since last odometry reset (milliseconds)
     */
    uint32_t getDriftTime() const;

private:
    uint32_t lastOdomReset;
    uint32_t lastAlertTime;
    std::string lastMessage;

    /**
     * @brief Display alert on controller
     */
    void show(const char* msg, Priority priority);

    /**
     * @brief Check if enough time has passed to show new alert
     * Prevents alert spam
     */
    bool canShowAlert() const;
};

} // namespace subsystems
