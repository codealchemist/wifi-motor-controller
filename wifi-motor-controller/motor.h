#ifndef Motor_h
#define Motor_h
#include "Arduino.h"
#include "AccelStepper.h"

class MotorHelper {
  public:
    MotorHelper(int DIR_PIN, int STEP_PIN, int REV_STEPS);
    void moveDegrees(AccelStepper stepper, int degrees);
    // int getStepsPosition();
    // int getDegreesPosition();
    // void test();
    // void run();
    // void moveTo(int steps);
    // int distanceToGo();
    // void setMaxSpeed(int speed);
    // void setSpeed(int speed);
    // void setAcceleration(int acceleration);
    // bool isRunning();
    // int currentPosition();
    // void setCurrentPosition(int steps);

  private:
    int DIR_PIN;
    int STEP_PIN;
    int REV_STEPS;
    int currentDegrees;
    void log(String message);
    // AccelStepper stepper;
};

#endif
