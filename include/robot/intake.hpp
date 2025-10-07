#pragma once
#include "pros/optical.hpp"
#include "pros/motors.hpp"
#include "pros/misc.hpp"
#include "pros/adi.hpp"


namespace subsystems {


enum class DONUT_COLOR { NONE, RED, BLUE };


class Intake {
public:
    Intake(std::vector<pros::Motor> imotors, int ipiston, int top_color_sensor_port, int ilimit);
    /**
     * @brief
     *
     * @param speed (0-1) multiplier for speed
     */
    void move_forward(float speed = 1);
    void move_backward();
    void move_relative(float position, float speed);
    void run(pros::controller_digital_e_t intakeButton, pros::controller_digital_e_t outtakeButton, pros::controller_digital_e_t pistonButton, pros::controller_digital_e_t killSwitch);
    void stop();
    pros::ADIDigitalOut getIntakePist();
    pros::ADIDigitalIn getLimitSwitch();
    bool is_active() const;
    void activate();
    void deactivate();
    void set_target_color(DONUT_COLOR color);
    DONUT_COLOR getTargetColour();
    int getSensorHue();
    double getPosition();

    void setPosition();
    int getMotorVelocity();
    void startIntakeTask();
    void startColourSort();
    void endIntakeTask();
    void piston_out();
    void piston_in();

private:
    pros::MotorGroup intake_motors;
    pros::ADIDigitalOut intake_pist;
    pros::ADIDigitalIn limitSwitch;
    pros::Optical top_color_sensor;  // Only keeping one color sensor
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

