#pragma once

#include "pros/imu.hpp"

namespace subsystems {

/**
 * @brief IMU drift characterization and compensation system
 *
 * Measures IMU bias at startup and applies compensation during autonomous.
 * Over 60 seconds, even small drift rates accumulate to significant heading errors.
 *
 * Usage:
 *   // At startup (robot must be still!)
 *   imuDrift.characterize();
 *
 *   // During autonomous
 *   imuDrift.startCompensation();
 *   // ... run autonomous ...
 *   float corrected_heading = imuDrift.getCompensatedHeading();
 */
class IMUDrift {
public:
    IMUDrift(pros::Imu* imu);

    /**
     * @brief Characterize IMU drift rate
     *
     * Robot MUST be completely still during this process!
     * Samples heading over 5 seconds to calculate drift rate.
     *
     * Call this during initialize() or competition_initialize().
     *
     * @return true if characterization succeeded, false if robot moved
     */
    bool characterize();

    /**
     * @brief Start compensation timer
     * Call this at the start of autonomous
     */
    void startCompensation();

    /**
     * @brief Get compensated heading
     *
     * @return Current heading with drift compensation applied
     */
    float getCompensatedHeading();

    /**
     * @brief Get raw IMU heading (no compensation)
     */
    float getRawHeading();

    /**
     * @brief Get measured drift rate
     * @return Drift rate in degrees per second
     */
    float getDriftRate() const { return driftRate; }

    /**
     * @brief Get total drift correction applied
     * @return Total correction in degrees
     */
    float getTotalCorrection() const;

    /**
     * @brief Check if characterization has been performed
     */
    bool isCharacterized() const { return characterized; }

    /**
     * @brief Reset compensation (call after setPose or wall alignment)
     */
    void resetCompensation();

private:
    pros::Imu* imu;
    float driftRate;           // Drift rate in deg/sec
    bool characterized;
    uint32_t compensationStart;
    float baselineHeading;
};

} // namespace subsystems
