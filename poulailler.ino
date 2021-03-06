
#include <DS3231M.h>
#include <avr/sleep.h>
#include <TinyStepper_28BYJ_48.h>
#include <EEPROM.h>


#define STATE_STARTUP         0
#define STATE_IDLE            1
#define STATE_DOORMANUMOTION  2
#define STATE_DOORAUTOMOTION  3

#define ACTION_NONE        0
#define ACTION_SETUPALARM  1
#define ACTION_ALARM       2
#define ACTION_BTN         3






#define ACTION_IDLEDOOR    0
#define ACTION_OPENDOOR    1
#define ACTION_CLOSEDOOR   2





//#define DEBUG
#ifdef DEBUG
#define DEBUG_INIT Serial.begin(115200)
#define DEBUG_PRINT(x)  Serial.print (x)
#define DEBUG_PRINTLN(x)  Serial.println (x)
#define DEBUG_PRINT_NUMBER(x,y) Serial.print (x,y)
#define DEBUG_PRINTLN_NUMBER(x,y) Serial.println (x,y)
#define DEBUG_DELAY(x) delay(x)
#else
#define DEBUG_INIT
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT_NUMBER(x,y)
#define DEBUG_PRINTLN_NUMBER(x,y)
#define DEBUG_DELAY(x)
#endif



long idleTime = 0;
int state = STATE_IDLE;
int action = ACTION_NONE;
byte btnAction = ACTION_CLOSEDOOR;
unsigned long btnActionStartTime = 0;



void setup() {
  DEBUG_INIT;

  setupRTC();
  setupRTCAlarm();
  setupBtn();
  setupDoor();

  setState(STATE_IDLE);
  setupNextAlarm();

  // set last btn performed action
  if(getCurrentPosition()==getHighPosition()){
    btnAction = ACTION_OPENDOOR;
  }
}

void loop() {
  switch (action) {
    case ACTION_ALARM:
      performAlarmAction();
      break;
    case ACTION_BTN:
      performBtnAction();
      break;
  }

  if (state == STATE_IDLE) {
    if (idleTime == 0) {
      idleTime = millis();
    } else if ((millis() - idleTime) > 15000) {
      poweroffDoor();
      enterSleep();
      idleTime = 0;
    }
  }

  if (state == STATE_DOORAUTOMOTION) {
    if (!moveDoor()) {
      stopDoor();
      setState(STATE_IDLE);
    }
  }
  if (state == STATE_DOORMANUMOTION) {
    if ( isBtnPushed() ) {
      moveDoor();
    } else {
      stopDoor();
      (btnAction == ACTION_OPENDOOR) ? setHighPosition() : setLowPosition();
      setState(STATE_IDLE);
      // reset btn
      setupBtn();
    }
  }
}

/***************************************************
   Name:        enterSleep
   Returns:     Nothing.
   Parameters:  None.
   Description: Enters the arduino into sleep mode.

***************************************************/
void enterSleep(void)
{
  DEBUG_PRINTLN(F("enterSleep"));
  DEBUG_DELAY(100);

  set_sleep_mode(SLEEP_MODE_PWR_SAVE);   /* EDIT: could also use SLEEP_MODE_PWR_DOWN for lowest power consumption. */
  sleep_enable();

  /* Now enter sleep mode. */
  sleep_mode();

  /* The program will continue from here after the WDT timeout*/
  sleep_disable(); /* First thing to do is disable sleep. */

  DEBUG_PRINTLN(F("out Sleep"));
}

/***************************************************
   Name:        setState
   Returns:     Nothing.
   Parameters:
                newState: new system state.
   Description: change the system state.

***************************************************/
void setState(int newState) {
  state = newState;
  if (newState != STATE_IDLE) {
    idleTime = 0;
  }
}


/***************************************************
   Name:        performAlarmAction
   Returns:     Nothing.
   Parameters:  Nothing.
   Description: perform the action corresponding to
                alarm action.

***************************************************/
void performAlarmAction() {
  DEBUG_PRINTLN(F("performAlarmAction"));
  // clean action
  action = ACTION_NONE;
  
  switch (state) {
    case STATE_IDLE:
      startMoveAutoDoor(getAlarmAction() == ACTION_OPENDOOR);
      // change system state
      setState(STATE_DOORAUTOMOTION);
      break;
    default:
      DEBUG_PRINT(F("performAlarmAction skipped "));
      DEBUG_PRINTLN(state);
      break;
  }
  setupNextAlarm();
}

/***************************************************
   Name:        performBtnAction
   Returns:     Nothing.
   Parameters:  Nothing.
   Description: perform the action corresponding to
                btn action according to system state

***************************************************/
void performBtnAction() {
  //DEBUG_PRINTLN(F("performBtnAction"));
  
  //prevente from switch rebond
  delay(300);

  
  switch (state) {
    /**
     * when system is in idle mode and :
     *    button is released before 2 seconds start an automatic move 
     *    button is pressed during more than 2 seconds switch to setup mode
     */
    case STATE_IDLE:
      // setup btn click time
      if (btnActionStartTime == 0) {
        btnActionStartTime = millis();
        btnAction = (btnAction == ACTION_OPENDOOR) ? ACTION_CLOSEDOOR : ACTION_OPENDOOR;
      }

      // btn is released
      if ( digitalRead(3) == HIGH ) { 
        // Automatic move
        startMoveAutoDoor(btnAction == ACTION_OPENDOOR);
        // change system state
        setState(STATE_DOORAUTOMOTION);
        // clean action
        action = ACTION_NONE;
        // reset btn click time
        btnActionStartTime = 0;
        // reset btn interuption mgr
        setupBtn();
      } else if ( millis() - btnActionStartTime > 2000 )  { // if still pushed
        // Manual move
        startMoveManuDoor(btnAction == ACTION_OPENDOOR);
        // change system state
        setState(STATE_DOORMANUMOTION);
        // clean action
        action = ACTION_NONE;
        // reset btn click time
        btnActionStartTime = 0;
      }
      break;
      /**
       * If btn is pushed while door moving
       * stop mvt and back to idle state
       */
    case STATE_DOORAUTOMOTION:
      // stop door mvt
      stopDoor();
      // back to idle
      setState(STATE_IDLE);
      // wait btn release before continue
      while ( digitalRead(3) == LOW) {};
      // clean action
      action = ACTION_NONE;
      // reset btn interuption mgr
      setupBtn();
      break;
    default:
      DEBUG_PRINT(F("performBtnAction skipped "));
      DEBUG_PRINTLN(state);
      break;
  }
  
}
