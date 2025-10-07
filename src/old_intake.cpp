// #include "intake.hpp"
// #include "pros/rtos.hpp"
// #include"robotConfig.hpp"

// double intakeDir = 0;

// double colourVal() {
//     return optical.get_hue();
// }

// void intakeControl(int colour) {
//   while (true) {
//     //if task has been turned on and 1 is to redirect red rings onto the wall stake arm mech
//     if (intakeRunning == true && colour == 1) {
//       if(optical.get_hue() < 20 && !controller.get_digital(pros::controller_digital_e_t::E_CONTROLLER_DIGITAL_L1)) {
//         pros::delay(275);
//         while(optical.get_hue() < 20){
//           intake.move_voltage(0);
//         }
//       }
//     }

//     //if task has been turned on and 2 is to redirect blue rings onto the wall stake arm mech
//     else if (intakeRunning == true && colour == 2) {
//       if(optical.get_hue() > 200) {
//         pros::delay(275);
//         while(optical.get_hue() > 200 && !controller.get_digital(pros::controller_digital_e_t::E_CONTROLLER_DIGITAL_L1)){
//           intake.move_voltage(0);
//         }
//       }
//     }
//     pros::delay(10);
//   }
// }