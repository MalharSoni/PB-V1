#pragma once

#include "pros/adi.hpp"
#include "pros/rtos.hpp"
#include "pros/misc.hpp"

// ============================================================================
// GENERIC PNEUMATIC COMPONENT
// ============================================================================
// This is a universal pneumatic toggle class that works for ANY pneumatic
// mechanism in ANY VEX game:
//   - Clamps (High Stakes)
//   - Doinkers (High Stakes)
//   - Wings (future games)
//   - Lifts (future games)
//   - Intakes (future games)
//
// STUDENTS: Just create instances with game-specific names!
// Example:
//   lib::Pneumatic clamp(CLAMP_PORT);
//   lib::Pneumatic wings(WINGS_PORT);
//   lib::Pneumatic blocker(BLOCKER_PORT);
// ============================================================================

namespace lib {

class Pneumatic {
public:
    /**
     * @brief Construct a pneumatic mechanism
     *
     * @param port ADI port (A-H)
     * @param initial_state Initial state (false = retracted, true = extended)
     */
    Pneumatic(int port, bool initial_state = false);

    // ========================================================================
    // GENERIC CONTROL METHODS
    // ========================================================================

    /**
     * @brief Toggle between extended and retracted
     */
    void toggle();

    /**
     * @brief Extend the pneumatic (set to true)
     * Alias: activate(), open(), deploy()
     */
    void extend();
    void activate();  // Alternative name
    void open();      // Alternative name
    void deploy();    // Alternative name

    /**
     * @brief Retract the pneumatic (set to false)
     * Alias: deactivate(), close(), stow()
     */
    void retract();
    void deactivate(); // Alternative name
    void close();      // Alternative name
    void stow();       // Alternative name

    /**
     * @brief Set pneumatic to a specific state
     *
     * @param state true = extended, false = retracted
     */
    void set(bool state);

    /**
     * @brief Get current state
     *
     * @return true if extended, false if retracted
     */
    bool getState() const;

    /**
     * @brief Run with controller button (auto-toggles on press)
     *
     * @param button Controller button to bind to
     *
     * Usage in opcontrol():
     *   clamp.run(DIGITAL_Y);
     */
    void run(pros::controller_digital_e_t button);

private:
    pros::ADIDigitalOut piston;
    bool state;
};

} // namespace lib
