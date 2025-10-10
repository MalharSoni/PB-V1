#include "robot/telemetry.hpp"
#include "globals.hpp"
#include "pros/misc.h"
#include <ctime>

namespace subsystems {

Telemetry::Telemetry() : logfile(nullptr), logging(false), startTime(0) {}

void Telemetry::init() {
    // Generate filename with timestamp
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    snprintf(currentFilename, sizeof(currentFilename), "/usd/telemetry_%02d%02d_%02d%02d%02d.csv",
             timeinfo->tm_mon + 1, timeinfo->tm_mday,
             timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    // Open file for writing
    logfile = fopen(currentFilename, "w");

    if (logfile == nullptr) {
        // Fallback to simple filename if timestamp fails
        snprintf(currentFilename, sizeof(currentFilename), "/usd/telemetry.csv");
        logfile = fopen(currentFilename, "w");
    }

    if (logfile != nullptr) {
        // Write CSV header
        fprintf(logfile, "time_ms,x,y,theta,lf_temp,lm_temp,rf_temp,rm_temp,lf_curr,lm_curr,rf_curr,rm_temp,battery_mv,velocity\n");
        fflush(logfile);

        // Close and reopen to force header to disk
        fclose(logfile);
        logfile = fopen(currentFilename, "a");

        if (logfile == nullptr) {
            printf("[Telemetry] ERROR: Failed to reopen\n");
            logging = false;
            return;
        }

        logging = true;
        startTime = pros::millis();

        printf("[Telemetry] Logging to %s\n", currentFilename);
    } else {
        printf("[Telemetry] ERROR: Failed to open log file\n");
        logging = false;
    }
}

void Telemetry::log() {
    if (!logging) {
        return;
    }

    // Get current pose
    lemlib::Pose pose = chassis.getPose();

    // Get motor temperatures (Celsius)
    float lf_temp = leftFrontMotor.get_temperature();
    float lm_temp = leftMidMotor.get_temperature();
    float rf_temp = rightFrontMotor.get_temperature();
    float rm_temp = rightMidMotor.get_temperature();

    // Get motor currents (milliamps)
    float lf_curr = leftFrontMotor.get_current_draw();
    float lm_curr = leftMidMotor.get_current_draw();
    float rf_curr = rightFrontMotor.get_current_draw();
    float rm_curr = rightMidMotor.get_current_draw();

    // Get battery voltage (millivolts)
    int32_t battery = pros::battery::get_voltage();

    // Calculate average velocity (RPM average of all motors)
    float lf_vel = leftFrontMotor.get_actual_velocity();
    float lm_vel = leftMidMotor.get_actual_velocity();
    float rf_vel = rightFrontMotor.get_actual_velocity();
    float rm_vel = rightMidMotor.get_actual_velocity();
    float avg_velocity = (abs(lf_vel) + abs(lm_vel) + abs(rf_vel) + abs(rm_vel)) / 4.0;

    // Get elapsed time
    uint32_t elapsed = pros::millis() - startTime;

    // Write complete line to CSV
    fprintf(logfile, "%u,%.2f,%.2f,%.2f,%.1f,%.1f,%.1f,%.1f,%.0f,%.0f,%.0f,%.0f,%d,%.1f\n",
            elapsed, pose.x, pose.y, pose.theta,
            lf_temp, lm_temp, rf_temp, rm_temp,
            lf_curr, lm_curr, rf_curr, rm_curr,
            battery, avg_velocity);

    fflush(logfile);

    // Periodic close/reopen every 5 samples to force data to disk
    static int log_counter = 0;
    log_counter++;
    if (log_counter % 5 == 0) {
        fclose(logfile);
        logfile = fopen(currentFilename, "a");
        if (logfile == nullptr) {
            logging = false;
        }
    }
}

void Telemetry::close() {
    if (logfile != nullptr) {
        fclose(logfile);
        logfile = nullptr;
    }
    logging = false;
    printf("[Telemetry] Log file closed\n");
}

} // namespace subsystems
