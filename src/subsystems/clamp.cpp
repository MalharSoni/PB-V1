#include "robot/clamp.hpp"
#include "globals.hpp"

namespace subsystems {

Clamp::Clamp(int port)
    : piston(port, false) {}

void Clamp::toggle() {
    piston_toggle = !piston_toggle;
    piston.set_value(piston_toggle);
}

void Clamp::clamp_stake() {
    piston_toggle = true;
    piston.set_value(true);
}

void Clamp::declamp_stake() {
    piston_toggle = false;
    piston.set_value(false);
}

bool Clamp::get_state() const {
    return piston_toggle;
}

void Clamp::run(pros::controller_digital_e_t button) {
    if (master.get_digital_new_press(button)) {
        toggle();
    }
}

}
