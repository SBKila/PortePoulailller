
// create the stepper motor object
TinyStepper_28BYJ_48 stepper;
// stepsByRevolution = (360°/StepAngle)*GearRatio = (360°/5.625°)*64
#define STEPS_BY_REVOLUTION 2048

void setupDoor() {
  stepper.connectToPins(4, 5, 6, 7);
  stepper.setSpeedInStepsPerSecond(600);
  stepper.setAccelerationInStepsPerSecondPerSecond(600);
  stepper.disableMotor();
}

boolean isMoving = false;
void moveAutoDoor() {
  if (isMoving) {
    DEBUG_PRINT(F("."));
    stepper.processMovement();
  } else {
    DEBUG_PRINTLN((alarmAction == STATE_OPENDOOR) ? F("OpenDoor") : F("CloseDoor"));


    stepper.setupStop();
    stepper.setupRelativeMoveInSteps((alarmAction == ACTION_OPENDOOR) ? -STEPS_BY_REVOLUTION : STEPS_BY_REVOLUTION);
    isMoving = true;
  }

  if (stepper.motionComplete()) {
    stepper.disableMotor();
    state = STATE_SETUPALARM;
    isMoving = false;
  }
}

void moveManuDoor(boolean up) {
  if (isMoving) {
    DEBUG_PRINT(F("."));
    stepper.processMovement();
    if (stepper.motionComplete()) {
      stepper.setupRelativeMoveInSteps((up) ? -10 * STEPS_BY_REVOLUTION : 10 * STEPS_BY_REVOLUTION);
    }
  } else {
    DEBUG_PRINTLN(F("moveManuDoor"));
    stepper.setupRelativeMoveInSteps((up) ? -10 * STEPS_BY_REVOLUTION : 10 * STEPS_BY_REVOLUTION);
    isMoving = true;
  }

}
void stopManuDoor() {
  stepper.setupStop();
  isMoving = false;
}

void poweroffDoor() {
  stepper.disableMotor();
}
