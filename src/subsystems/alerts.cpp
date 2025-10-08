#include "robot/alerts.hpp"
#include "globals.hpp"
#include "pros/motors.h"

namespace subsystems {

Alerts::Alerts() : lastOdomReset(0), lastAlertTime(0), lastMessage("") {}

void Alerts::check() {
    // ========================================================================
    // MOTOR TEMPERATURE MONITORING
    // ========================================================================
    float lf_temp = leftFrontMotor.get_temperature();
    float lm_temp = leftMidMotor.get_temperature();
    float rf_temp = rightFrontMotor.get_temperature();
    float rm_temp = rightMidMotor.get_temperature();

    // Critical temperature (>60°C - motor protection kicks in at 55-60°C)
    if (lf_temp > 60) show("L15 CRITICAL!", CRITICAL);
    else if (lm_temp > 60) show("L14 CRITICAL!", CRITICAL);
    else if (rf_temp > 60) show("R16 CRITICAL!", CRITICAL);
    else if (rm_temp > 60) show("R13 CRITICAL!", CRITICAL);
    // Warning temperature (>55°C)
    else if (lf_temp > 55) show("L15 HOT", WARNING);
    else if (lm_temp > 55) show("L14 HOT", WARNING);
    else if (rf_temp > 55) show("R16 HOT", WARNING);
    else if (rm_temp > 55) show("R13 HOT", WARNING);

    // ========================================================================
    // MOTOR DISCONNECTION DETECTION
    // ========================================================================
    // Check for abnormal temperature readings (PROS_ERR_F = floating point error)
    // or zero current draw when motor should be running
    float lf_curr = leftFrontMotor.get_current_draw();
    float lm_curr = leftMidMotor.get_current_draw();
    float rf_curr = rightFrontMotor.get_current_draw();
    float rm_curr = rightMidMotor.get_current_draw();

    if (lf_temp == PROS_ERR || (lf_temp == 0 && lf_curr == 0))
        show("L15 DISC!", CRITICAL);
    else if (lm_temp == PROS_ERR || (lm_temp == 0 && lm_curr == 0))
        show("L14 DISC!", CRITICAL);
    else if (rf_temp == PROS_ERR || (rf_temp == 0 && rf_curr == 0))
        show("R16 DISC!", CRITICAL);
    else if (rm_temp == PROS_ERR || (rm_temp == 0 && rm_curr == 0))
        show("R13 DISC!", CRITICAL);

    // ========================================================================
    // BATTERY MONITORING
    // ========================================================================
    int32_t battery_mv = pros::battery::get_voltage();

    if (battery_mv < 11000) {
        show("BAT CRIT!", CRITICAL);  // <11.0V - nearly dead
    } else if (battery_mv < 11500) {
        show("BAT LOW!", WARNING);     // <11.5V - consider swapping
    }

    // ========================================================================
    // IMU STATUS MONITORING
    // ========================================================================
    pros::c::imu_status_e_t imu_status = inertial.get_status();

    if (imu_status & pros::c::E_IMU_STATUS_ERROR) {
        show("IMU ERROR!", CRITICAL);
    } else if (imu_status & pros::c::E_IMU_STATUS_CALIBRATING) {
        show("IMU CAL...", INFO);
    }

    // ========================================================================
    // ODOMETRY DRIFT WARNING
    // ========================================================================
    uint32_t drift_time = getDriftTime();

    // Warn if no odometry reset in 15 seconds
    if (drift_time > 15000) {
        show("ODOM DRIFT", WARNING);
    }

    // ========================================================================
    // SUBSYSTEM MOTOR MONITORING (Intake)
    // ========================================================================
    float is1_temp = intakeStage1.get_temperature();
    float is2_temp = intakeStage2.get_temperature();
    float is3_temp = intakeStage3.get_temperature();

    if (is1_temp > 60) show("INT1 HOT!", CRITICAL);
    else if (is2_temp > 60) show("INT2 HOT!", CRITICAL);
    else if (is3_temp > 60) show("INT3 HOT!", CRITICAL);
}

void Alerts::resetDriftTimer() {
    lastOdomReset = pros::millis();
}

uint32_t Alerts::getDriftTime() const {
    return pros::millis() - lastOdomReset;
}

void Alerts::show(const char* msg, Priority priority) {
    // Prevent alert spam - only show new alert every 2 seconds
    if (!canShowAlert() && lastMessage == msg) {
        return;
    }

    // Display on controller
    master.print(0, 0, "%-16s", msg);  // Left-aligned, 16 chars

    // Rumble pattern based on priority
    switch (priority) {
        case CRITICAL:
            master.rumble("---");  // Long triple pulse
            break;
        case WARNING:
            master.rumble("-");    // Single pulse
            break;
        case INFO:
            // No rumble for info
            break;
    }

    // Update last alert tracking
    lastAlertTime = pros::millis();
    lastMessage = msg;

    // Log to console
    const char* priority_str[] = {"INFO", "WARN", "CRIT"};
    printf("[Alert %s] %s\n", priority_str[priority], msg);
}

bool Alerts::canShowAlert() const {
    // Allow new alert every 2 seconds
    return (pros::millis() - lastAlertTime) > 2000;
}

} // namespace subsystems
