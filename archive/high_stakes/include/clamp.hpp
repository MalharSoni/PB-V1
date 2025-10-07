#pragma once

#include "pros/adi.hpp"
#include "pros/rtos.hpp"
#include "pros/misc.hpp"

namespace subsystems {

class Clamp {
public:
    Clamp(int port);
    void toggle();
    void run(pros::controller_digital_e_t button);
    void clamp_stake();
    void declamp_stake();
    bool get_state() const;

private:
    pros::ADIDigitalOut piston;
    bool piston_toggle = false;
};

}
