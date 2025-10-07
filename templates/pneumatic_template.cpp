// ============================================================================
// PNEUMATIC TEMPLATE - Copy-Paste Example
// ============================================================================
// This template shows how to use lib::Pneumatic for ANY pneumatic mechanism
// in ANY VEX game.
//
// STEPS TO USE:
// 1. Copy this code to your globals.cpp
// 2. Rename the variable to match YOUR game mechanism
// 3. Update the port number in globals.hpp
// 4. Add controller mapping in main.cpp
//
// That's it! No need to create a custom class.
// ============================================================================

#include "lib/Pneumatic.hpp"
#include "globals.hpp"

// ============================================================================
// EXAMPLE 1: High Stakes - Mobile Goal Clamp
// ============================================================================
// In globals.hpp:
//   #define CLAMP_PORT 'A'
//   extern lib::Pneumatic clamp;
//
// In globals.cpp:
lib::Pneumatic clamp(CLAMP_PORT);
//
// In main.cpp opcontrol():
//   clamp.run(DIGITAL_Y);  // Y button toggles clamp

// ============================================================================
// EXAMPLE 2: Future Game - Wings
// ============================================================================
// In globals.hpp:
//   #define WINGS_PORT 'B'
//   extern lib::Pneumatic wings;
//
// In globals.cpp:
lib::Pneumatic wings(WINGS_PORT);
//
// In main.cpp opcontrol():
//   wings.run(DIGITAL_X);  // X button toggles wings
//
// Or manually control:
//   if (master.get_digital(DIGITAL_LEFT)) {
//       wings.extend();  // or wings.activate() or wings.deploy()
//   }
//   if (master.get_digital(DIGITAL_RIGHT)) {
//       wings.retract();  // or wings.deactivate() or wings.stow()
//   }

// ============================================================================
// EXAMPLE 3: Custom Initial State
// ============================================================================
// Start extended instead of retracted:
lib::Pneumatic blocker(BLOCKER_PORT, true);  // true = start extended

// ============================================================================
// AVAILABLE METHODS
// ============================================================================
// lib::Pneumatic mechanism(port);
//
// Methods (all do the same thing, use what reads best):
//   mechanism.toggle();           // Toggle state
//
//   mechanism.extend();            // Set to extended
//   mechanism.activate();          // (alias)
//   mechanism.open();              // (alias)
//   mechanism.deploy();            // (alias)
//
//   mechanism.retract();           // Set to retracted
//   mechanism.deactivate();        // (alias)
//   mechanism.close();             // (alias)
//   mechanism.stow();              // (alias)
//
//   mechanism.set(true);           // Set to specific state
//   mechanism.getState();          // Get current state (true/false)
//
//   mechanism.run(DIGITAL_BUTTON); // Toggle on button press (use in loop)

// ============================================================================
// AUTONOMOUS USAGE
// ============================================================================
// In autonomous routines (auton.cpp):
//
// void myAuton() {
//     clamp.extend();        // Grab mobile goal
//     pros::delay(500);      // Wait for clamp
//
//     // ... drive to scoring location ...
//
//     clamp.retract();       // Release mobile goal
// }

// ============================================================================
// TROUBLESHOOTING
// ============================================================================
// Problem: Pneumatic doesn't move
//   - Check ADI port is correct in globals.hpp
//   - Verify pneumatic is plugged into correct port
//   - Check air pressure is sufficient
//
// Problem: Pneumatic moves backwards
//   - Swap the tubing on the pneumatic solenoid
//   OR
//   - Use opposite method (extend instead of retract)
//
// Problem: Pneumatic toggles too fast
//   - Use run() method instead of manual toggle
//   - run() has built-in debouncing via get_digital_new_press()
