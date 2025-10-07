#include "lib/Pneumatic.hpp"
#include "globals.hpp"

namespace lib {

Pneumatic::Pneumatic(int port, bool initial_state)
    : piston(port, initial_state), state(initial_state) {}

void Pneumatic::toggle() {
    state = !state;
    piston.set_value(state);
}

void Pneumatic::extend() {
    state = true;
    piston.set_value(true);
}

void Pneumatic::activate() {
    extend();  // Alias for extend()
}

void Pneumatic::open() {
    extend();  // Alias for extend()
}

void Pneumatic::deploy() {
    extend();  // Alias for extend()
}

void Pneumatic::retract() {
    state = false;
    piston.set_value(false);
}

void Pneumatic::deactivate() {
    retract();  // Alias for retract()
}

void Pneumatic::close() {
    retract();  // Alias for retract()
}

void Pneumatic::stow() {
    retract();  // Alias for retract()
}

void Pneumatic::set(bool new_state) {
    state = new_state;
    piston.set_value(state);
}

bool Pneumatic::getState() const {
    return state;
}

void Pneumatic::run(pros::controller_digital_e_t button) {
    if (master.get_digital_new_press(button)) {
        toggle();
    }
}

} // namespace lib
