#pragma once

#include "pros/adi.hpp"
#include "pros/rtos.hpp"
#include "pros/misc.hpp"

namespace subsystems {

class Doinker {
public:
    Doinker(int port);
    void toggle();
    void run(pros::controller_digital_e_t button);
    void activate();
    void deactivate();
    bool get_state() const;

private:
    pros::ADIDigitalOut piston;
    bool piston_toggle = false;
};

}
