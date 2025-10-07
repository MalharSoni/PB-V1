#include "robot/doinker.hpp"
#include "globals.hpp"

namespace subsystems {

Doinker::Doinker(int port)
    : piston(port, false) {}

void Doinker::toggle() {
    piston_toggle = !piston_toggle;
    piston.set_value(piston_toggle ? 1 : 0);
}

void Doinker::activate() {
    piston_toggle = true;
    piston.set_value(1);
}

void Doinker::deactivate() {
    piston_toggle = false;
    piston.set_value(0);
}

bool Doinker::get_state() const {
    return piston_toggle;
}

void Doinker::run(pros::controller_digital_e_t button) {
    if (master.get_digital_new_press(button)) {
        toggle();
    }
}

}
