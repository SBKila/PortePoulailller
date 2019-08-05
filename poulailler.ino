
#include <DS3231M.h>
#include <avr/sleep.h>
#include <TinyStepper_28BYJ_48.h>



#define STATE_IDLE        0
#define STATE_SETUPALARM  1
#define STATE_ALARMACTION 2
#define STATE_BTNACTION   3

#define ACTION_OPENDOOR    0
#define ACTION_CLOSEDOOR   1


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
int state;


void setup() {
  DEBUG_INIT;

  setupRTC();
  setupRTCAlarm();
  setupBtn();
  setupDoor();

  state = STATE_SETUPALARM;
}




void loop() {
  if (state == STATE_IDLE) {
    if (idleTime == 0) {
      idleTime = millis();
    } else if ((millis() - idleTime) > 10000) {
      poweroffDoor();
      enterSleep();
    }
  } else {
    idleTime = 0;
    if (state == STATE_SETUPALARM) {
      setupNextAlarm();
    }
    if (state == STATE_ALARMACTION) {
      moveAutoDoor();
    }
    if ( state == STATE_BTNACTION) {
      performBtnAction();
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
