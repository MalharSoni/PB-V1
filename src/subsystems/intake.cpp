#include "robot/intake.hpp"
#include "globals.hpp"
#include "pros/adi.h"
#include "robot/arm.hpp"
#include <iostream>

namespace subsystems {

Intake::Intake(std::vector<pros::Motor> imotors, int ipiston,
               int top_color_sensor_port, int ilimit)
    : intake_motors(imotors), intake_pist(ipiston, true),
      top_color_sensor(top_color_sensor_port, 3),
      target_color(DONUT_COLOR::NONE), limitSwitch(ilimit) {}

void Intake::move_forward(float speed) {
  intake_motors.move_voltage(12000 * speed);
}
void Intake::move_relative(float position, float speed) {
  intake_motors.move_relative(position, speed);
}

void Intake::move_backward() { intake_motors.move_voltage(-12000); }

void Intake::stop() { intake_motors.move_voltage(0); }

bool Intake::is_active() const { return active; }

void Intake::activate() {
  active = true;
  move_forward();
}

void Intake::deactivate() {
  endIntakeTask();
  active = false;
  stop();
}
void Intake::piston_out() { intake_pist.set_value(false); }

void Intake::piston_in() { intake_pist.set_value(true); }

void Intake::set_target_color(DONUT_COLOR color) { target_color = color; }

DONUT_COLOR Intake::getTargetColour() { return target_color; }

int Intake::getSensorHue() { return top_color_sensor.get_hue(); }

double Intake::getPosition() { return intake_motors.get_positions().at(0); }

void Intake::setPosition() { intake_motors.set_zero_position(0); }

pros::ADIDigitalOut Intake::getIntakePist() { return intake_pist; }

pros::ADIDigitalIn Intake::getLimitSwitch() { return limitSwitch; }

int Intake::getMotorVelocity() {
  return (int)intake_motors.get_actual_velocities()[0];
  // return (int)intake_motors.get_voltages()[0];
}

void Intake::startIntakeTask() {
  if (!isIntakeTaskRunning && !isColourSortRunning) {
    std::cout << "task started\n";
    intakeTask =
        pros::c::task_create(intakeFunction, this, TASK_PRIORITY_DEFAULT,
                             TASK_STACK_DEPTH_DEFAULT, "Colour Sorting");
    isIntakeTaskRunning = true;
  }
}

void Intake::startColourSort() {
  if (!isIntakeTaskRunning && !isColourSortRunning) {
    std::cout << "task started\n";
    top_color_sensor.set_led_pwm(100);
    colourSortTask =
        pros::c::task_create(colourSort, this, TASK_PRIORITY_DEFAULT,
                             TASK_STACK_DEPTH_DEFAULT, "Colour Sorting");
    isColourSortRunning = true;
  }
}

void Intake::endIntakeTask() {
  if (isIntakeTaskRunning) {
    std::cout << "task ended\n";
    pros::c::task_delete(intakeTask);
    isIntakeTaskRunning = false;
    deactivate();
  }

  if (isColourSortRunning) {
    std::cout << "task ended\n";
    top_color_sensor.set_led_pwm(0);
    pros::c::task_delete(colourSortTask);
    isColourSortRunning = false;
    deactivate();
  }
}

void Intake::run(pros::controller_digital_e_t intakeButton,
                 pros::controller_digital_e_t outtakeButton,
                 pros::controller_digital_e_t pistonButton,
                 pros::controller_digital_e_t killSwitch) {
  if (master.get_digital(intakeButton)) {
    startColourSort();
    active = true;
  } else if (master.get_digital(outtakeButton)) {
    move_backward();
    active = true;
  } else if (active) {
    deactivate();
    active = false;
  }

  if (master.get_digital(killSwitch)) {
    intake.set_target_color(DONUT_COLOR::NONE);
  }

  if (master.get_digital_new_press(pistonButton)) {
    up = !up;
    intake_pist.set_value(up ? 1 : 0);
  }
  /*
  if (active && target_color != DONUT_COLOR::NONE) {
      int top_hue = top_color_sensor.get_hue();

      if(target_color == DONUT_COLOR::BLUE){
          if(top_hue < 30) {
              while(optical.get_hue() < 30 &&
  !controller.get_digital(outtakeButton)) { intake.move_voltage(0);
              }
          }
      }
      if (target_color == DONUT_COLOR::BLUE && top_hue > 200) {
          while(optical.get_hue() > 200 &&
  !controller.get_digital(outtakeButton)) { intake.move_voltage(0);
          }
      }
  }
  */
}

void colourSort(void *iintake) {
  Intake *intake = (Intake *)iintake;
  intake->move_forward();
  int pause = 0;
  double startPos = 0;
  bool ourRing = false;
  while (true) {
    intake->move_forward();
    pros::lcd::print(5, "colour", intake->getSensorHue());

    if (intake->getTargetColour() == DONUT_COLOR::RED) {
      std::cout << "thing\n";
      pros::lcd::print(6, "RED");
      if ((intake->getSensorHue() <= 240 && intake->getSensorHue() >= 200)) {
        // pros::delay(80);
        // pros::lcd::print(7, "LIMIT DETECTED");
        // intake->stop();
        // pros::delay(100);
        ourRing = false;
      }

    //   if ((intake->getSensorHue() <= 13 && intake->getSensorHue() >= 1) ||
    //       (intake->getSensorHue() <= 359 && intake->getSensorHue() >= 310)) {
    //     ourRing = true;
    //   }
    else {
        ourRing=true;
    }

      while (!ourRing && intake->getLimitSwitch().get_value() == HIGH) {
        pros::delay(60);
        intake->move_backward();
        pros::delay(120);
      }
    }


    if (intake->getTargetColour() == DONUT_COLOR::BLUE) {
        std::cout << "thing\n";
        pros::lcd::print(6, "RED");
        

        if ((intake->getSensorHue() <= 13 && intake->getSensorHue() >= 1) ||
            (intake->getSensorHue() <= 359 && intake->getSensorHue() >= 310)) {
          ourRing = false;
        } else {
            ourRing = true;
        }

        // if ((intake->getSensorHue() <= 240 && intake->getSensorHue() >= 200)) {
        //     // pros::delay(80);
        //     // pros::lcd::print(7, "LIMIT DETECTED");
        //     // intake->stop();
        //     // pros::delay(100);
        //     ourRing = true;
        //   }

        

        while (!ourRing && intake->getLimitSwitch().get_value() == HIGH) {
          pros::delay(60);
          intake->move_backward();
          pros::delay(120);
        }
    }
    intake->move_forward();
    pros::delay(5);

    // anti jam KEEP
    if (abs(intake->getMotorVelocity()) < 10) {
      if (arm.isPickup == false) {
        intake->move_forward(-1);
        pros::delay(100);
        std::cout << "thing\n";
        // intake->move_relative(1, -5);
      }
    }
    std::cout << intake->getSensorHue() << "\n";
    // pros::delay(1);
  }
}

void intakeFunction(void *iintake) {
  Intake *intake = (Intake *)iintake;
  intake->move_forward();
  while (true) {
    intake->move_forward();
    pros::delay(50);
    std::cout << intake->getMotorVelocity() << " speed\n";
    if (abs(intake->getMotorVelocity()) < 5) {
      if (arm.isPickup == false) {
        // intake->move_forward(-1);
        // pros::delay(15);
        // std::cout << "thing\n";
        intake->move_relative(1, -5);
      }
    }
  }
}

} // namespace subsystems
