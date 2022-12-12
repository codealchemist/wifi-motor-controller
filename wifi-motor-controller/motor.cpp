// VREF for Nema 17HS8401 1.7A:
// 1.5 * 8 * 0.1 = 1.2vref ... MAX recommended vref (10% margin)
// 1 * 8 * 0.1 = 0.8vref ... 1vref still OK!
// 0.6vref * 2.5 = 1.5A --> 0.6vref (alt calculation)

#include "Arduino.h"
#include "motor.h"
#include "AccelStepper.h"

MotorHelper::MotorHelper(int _DIR_PIN, int _STEP_PIN, int _REV_STEPS) {
  REV_STEPS = _REV_STEPS;

  // stepper = _stepper;
  // stepper.setMaxSpeed(1000);
  // stepper.setAcceleration(60);
  // stepper.setSpeed(400);
  // stepper.setCurrentPosition(0);
}

void MotorHelper::log(String message) {
  Serial.println("[ Motor ] " + message);
}

void MotorHelper::moveDegrees(AccelStepper stepper1, int degrees) {
  Serial.println("moveDegrees: " + String(degrees));
  int steps = degrees * 200 / 360;

  Serial.println("Move steps: " + String(steps));
  stepper1.moveTo(steps);
  Serial.println("Movement completed!");
}

// int MotorHelper::getDegreesPosition() {
//   int steps = stepper.currentPosition();
//   int degrees = steps * 360 / 200;
//   return degrees;
// }
