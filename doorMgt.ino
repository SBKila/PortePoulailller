
// create the stepper motor object
TinyStepper_28BYJ_48 stepper;
// stepsByRevolution = (360°/StepAngle)*GearRatio = (360°/5.625°)*64
#define STEPS_BY_REVOLUTION 2048
#define FULL_OPEN_REVOLUTION 6

#define TAG_OFFSET 0
long HIGHPOSITION_OFFSET = sizeof(byte);
long CURRENTPOSITION_OFFSET = sizeof(byte) + sizeof(long);

long lowPosition = -1;
long highPosition = -1;

long getLowPosition() {
  return lowPosition;
}
void setLowPosition() {
  lowPosition = stepper.getCurrentPositionInSteps();
  if (highPosition != 0) {
    highPosition = highPosition - lowPosition;
    EEPROM.put(HIGHPOSITION_OFFSET,highPosition);
    DEBUG_PRINT(F("setHighPosition "));DEBUG_PRINTLN(highPosition);
  }
  stepper.setCurrentPositionInSteps(0);
  EEPROM.put(CURRENTPOSITION_OFFSET,0);
  lowPosition = 0;
  DEBUG_PRINT(F("setLowPosition "));DEBUG_PRINTLN(lowPosition);
}

long getHighPosition() {
  return highPosition;
}
void setHighPosition() {
  if (lowPosition == 0) {
    highPosition = stepper.getCurrentPositionInSteps();
    EEPROM.put(HIGHPOSITION_OFFSET,highPosition);
    EEPROM.put(TAG_OFFSET,187);
    DEBUG_PRINT(F("setHighPosition "));DEBUG_PRINTLN(highPosition);
  }
}

long getCurrentPosition(){
  return stepper.getCurrentPositionInSteps();
}

void setupDoor() {
  DEBUG_PRINT(F("setupDoor :"));
  
  stepper.connectToPins(4, 5, 6, 7);
  stepper.setSpeedInStepsPerSecond(600);
  stepper.setAccelerationInStepsPerSecondPerSecond(600);
  stepper.disableMotor();

  byte tag;
  EEPROM.get(TAG_OFFSET,tag);
  if(tag == 187){
    lowPosition=0;
    EEPROM.get(HIGHPOSITION_OFFSET,highPosition);
    DEBUG_PRINT(F(" load HighPosition "));DEBUG_PRINT(highPosition);
    long currentPosition=0;
    EEPROM.get(CURRENTPOSITION_OFFSET,currentPosition);
    stepper.setCurrentPositionInSteps(currentPosition);
    DEBUG_PRINT(F(" load CurrentPosition "));DEBUG_PRINT(currentPosition);
  }
  DEBUG_PRINTLN(F(""));
}

void startMoveManuDoor(boolean up) {
  DEBUG_PRINT(F("startMoveManuDoor "));
  startMoveDoor(up,true);
}
void startMoveAutoDoor(boolean up) {
  DEBUG_PRINT(F("startMoveAutoDoor "));
  startMoveDoor(up,false);
}
void startMoveDoor(boolean up, boolean force) {
  DEBUG_PRINTLN(up?F("OPEN"):F("CLOSE"));
  // stop ongoing movement
  stepper.setupStop();

  // setup Full door movement
  if ((lowPosition != -1) && (highPosition != -1) && !force) {
    stepper.setupMoveInSteps(up ? highPosition : lowPosition);
  } else {
    stepper.setupRelativeMoveInSteps(up ? FULL_OPEN_REVOLUTION * STEPS_BY_REVOLUTION : -FULL_OPEN_REVOLUTION * STEPS_BY_REVOLUTION);
  }
}

boolean moveDoor() {
  //DEBUG_PRINT(F("."));
  stepper.processMovement();
  EEPROM.put(CURRENTPOSITION_OFFSET,stepper.getCurrentPositionInSteps());
  //DEBUG_PRINT(F("setCurrent Position "));DEBUG_PRINTLN(stepper.getCurrentPositionInSteps());

  if (stepper.motionComplete()) {
    return false;
  }
  return true;
}

void stopDoor() {
  DEBUG_PRINT(F("stopDoor at "));
  DEBUG_PRINTLN(stepper.getCurrentPositionInSteps());
  // stop ongoing movement
  stepper.setupStop();
}

void poweroffDoor() {
  stepper.disableMotor();
}
