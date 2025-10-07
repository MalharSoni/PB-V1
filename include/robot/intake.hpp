#pragma once
#include "lib/MotorSubsystem.hpp"
#include "pros/optical.hpp"
#include "pros/motors.hpp"
#include "pros/misc.hpp"
#include "pros/adi.hpp"


namespace subsystems {


enum class DONUT_COLOR { NONE, RED, BLUE };


class Intake : public lib::MotorSubsystem {
public:
    Intake(std::vector<pros::Motor> imotors, int ipiston, int top_color_sensor_port, int ilimit);

    // ========================================================================
    // MOTOR CONTROL (uses inherited MotorSubsystem methods)
    // ========================================================================

    /**
     * @brief Move intake forward at specified speed
     * @param speed (0-1) multiplier for speed, defaults to full speed
     */
    void move_forward(float speed = 1);

    /**
     * @brief Move intake backward at full speed
     */
    void move_backward();

    // Inherited from MotorSubsystem:
    //   - moveRelative(float delta, float speed)
    //   - stop()
    //   - getPosition()
    //   - setZeroPosition(float position = 0)
    //   - getVelocity()

    /**
     * @brief Get motor velocity (wrapper for getVelocity())
     * @return Velocity in RPM
     */
    int getMotorVelocity();

    // ========================================================================
    // GAME-SPECIFIC: HIGH STAKES COLOR SORTING
    // ========================================================================

    void run(pros::controller_digital_e_t intakeButton, pros::controller_digital_e_t outtakeButton, pros::controller_digital_e_t pistonButton, pros::controller_digital_e_t killSwitch);
    pros::ADIDigitalOut getIntakePist();
    pros::ADIDigitalIn getLimitSwitch();
    bool is_active() const;
    void activate();
    void deactivate();
    void set_target_color(DONUT_COLOR color);
    DONUT_COLOR getTargetColour();
    int getSensorHue();
    void startIntakeTask();
    void startColourSort();
    void endIntakeTask();
    void piston_out();
    void piston_in();

private:
    // Game-specific hardware (High Stakes)
    pros::ADIDigitalOut intake_pist;
    pros::ADIDigitalIn limitSwitch;
    pros::Optical top_color_sensor;

    // State tracking
    bool active = false;
    bool up = false;
    bool isIntakeTaskRunning = false;
    bool isColourSortRunning = false;
    pros::task_t colourSortTask;
    pros::task_t intakeTask;
    DONUT_COLOR target_color;
};


void colourSort(void* iintake);
void intakeFunction(void* iintake);


}

