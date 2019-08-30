DS3231M_Class DS3231M;


//#define DEBUG_TIMER
#define DS3231M_INTERRUPTION_PIN 2

#ifdef DEBUG_TIMER
byte actiondebug = ACTION_CLOSEDOOR;
#endif

//sun calendar for marseille
//////////////////////////    |------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
//////////////////////////    | JANV |  FEV  |  Mars |  Avr  |  Mai  |  Jun  |  Juil |  Aout |  Sept |  Oct  |  Nov  |  Dec  |
//////////////////////////    |15  30| 15  30| 15  30| 15  30| 15  30| 15  30| 15  30| 15  30| 15  30| 15  30| 15  30| 15  30|
//////////////////////////    |------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
int SunRiseGTMHours[24]   = {   7,  7,  6,  6,  5,  5,  5,  4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  6,  6,  6,  7,  7  };
int SunRiseGTMMinutes[24] = {  10, 00, 40, 20, 55, 25,  0, 35, 20,  5,  0,  5, 20, 30, 45,  5, 20, 40, 55, 15, 35, 55, 10, 15  };
int SunSetGTMHours[24]    = {  16, 16, 17, 17, 17, 18, 18, 18, 18, 19, 19, 19, 19, 19, 18, 18, 17, 17, 16, 16, 16, 16, 16, 16  };
int SunSetGTMMinutes[24]  = {  25, 45,  5, 20, 40, 00, 20, 35, 50,  5, 15, 20, 10,  0, 40, 15, 45, 20, 55, 30, 10,  0,  0, 10  };


#define DOORTIMEHOUROFFSET 0
#define DOORTIMEMINUTEOFFSET 30


byte alarmAction = ACTION_NONE;


void setupRTC() {
  DEBUG_PRINT(F("setupRTC : "));
  while (!DS3231M.begin()) {                                                  // Initialize RTC communications    //
    DEBUG_PRINTLN(F("Unable to find DS3231MM. Checking again in 3s."));       // Show error text                  //
    delay(3000);                                                              // wait a second                    //
  } // of loop until device is located

  DateTime now = DS3231M.now();
  DEBUG_PRINT(F("Date/Time : ")); DEBUG_PRINT(now.year()); DEBUG_PRINT(F("-")); DEBUG_PRINT(now.month()); DEBUG_PRINT(F("-")); DEBUG_PRINT(now.day()); DEBUG_PRINT(F(" ")); DEBUG_PRINT(now.hour()); DEBUG_PRINT(F(":")); DEBUG_PRINT(now.minute()); DEBUG_PRINT(F(".")); DEBUG_PRINT(now.second());
  DEBUG_PRINT(F(" DS3231M chip temp:")); DEBUG_PRINT_NUMBER(DS3231M.temperature() / 100.0, 1); DEBUG_PRINTLN("\xC2\xB0""C");

  DS3231M.pinSquareWave();                                                      // setup pin as a 1Hz square signal //
}

void setupRTCAlarm() {
  pinMode(DS3231M_INTERRUPTION_PIN, INPUT_PULLUP);
}

void onAlarm(void) {
  cli();
  detachInterrupt(digitalPinToInterrupt(2));
  sleep_disable();
  action = ACTION_ALARM;
  sei();
}

byte getAlarmAction() {
  return alarmAction;
}


bool isBefore(TimeSpan a, TimeSpan b) {
  return (b - a).totalseconds() > 0;
}
bool isBeforeOrEqual(TimeSpan a, TimeSpan b) {
  return (b - a).totalseconds() >= 0;
}
TimeSpan addDoorActionDelay(TimeSpan a) {
  return a + TimeSpan(0, DOORTIMEHOUROFFSET, DOORTIMEMINUTEOFFSET, 0);
}

/**
   compute the half month date index
*/
byte getDateIndex(byte day, byte month) {
  return (month - 1) * 2 + (day > 15 ? 1 : 0);
}

void setupNextAlarm() {
  DEBUG_PRINT(F("setupNextAlarm : "));

  DateTime now = DS3231M.now();
  DEBUG_PRINT(F("it is ")); DEBUG_PRINT(now.hour()); DEBUG_PRINT(F("h")); DEBUG_PRINT(now.minute()); DEBUG_PRINT(F("m"));


  // compoute door today door open and close time
  TimeSpan sunRiseGTM = getSunGTMCalendar(now, true);
  DEBUG_PRINT(F(" sunRiseGTM:")); DEBUG_PRINT(sunRiseGTM.hours()); DEBUG_PRINT(F("h")); DEBUG_PRINT(sunRiseGTM.minutes()); DEBUG_PRINT(F("m")); DEBUG_PRINT(sunRiseGTM.seconds());DEBUG_PRINT(" "); DEBUG_PRINT(sunRiseGTM.totalseconds());
  TimeSpan sunSetGTM = getSunGTMCalendar(now, false);
  DEBUG_PRINT(F(" sunSetGTM:")); DEBUG_PRINT(sunSetGTM.hours()); DEBUG_PRINT(F("h")); DEBUG_PRINT(sunSetGTM.minutes());DEBUG_PRINT(" "); DEBUG_PRINT(sunSetGTM.totalseconds());
  TimeSpan doorOpenGTM = addDoorActionDelay(sunRiseGTM);
  DEBUG_PRINT(F(" doorOpenGTM:")); DEBUG_PRINT(doorOpenGTM.hours()); DEBUG_PRINT(F("h")); DEBUG_PRINT(doorOpenGTM.minutes()); DEBUG_PRINT(" "); DEBUG_PRINT(doorOpenGTM.totalseconds());
  TimeSpan doorCloseGTM = addDoorActionDelay(sunSetGTM);
  DEBUG_PRINT(F(" doorCloseGTM:")); DEBUG_PRINT(doorCloseGTM.hours()); DEBUG_PRINT(F("h")); DEBUG_PRINT(doorCloseGTM.minutes());; DEBUG_PRINT(" "); DEBUG_PRINT(doorCloseGTM.totalseconds());

  // evaluate next action open/close
  TimeSpan currentTime(0, now.hour(), now.minute(), 0);
  DEBUG_PRINTLN("");
  DEBUG_PRINT(F(" currentTime:")); DEBUG_PRINT(currentTime.hours()); DEBUG_PRINT(F("h")); DEBUG_PRINT(currentTime.minutes()); DEBUG_PRINT(" "); DEBUG_PRINT(currentTime.totalseconds());
  if((!isBefore( currentTime,doorOpenGTM)) && isBefore(currentTime, doorCloseGTM)) {
    alarmAction = ACTION_CLOSEDOOR;
  } else {
    alarmAction = ACTION_OPENDOOR;
  }

#ifdef DEBUG_TIMER
  DEBUG_PRINT(F(" alarm at ")); DEBUG_PRINT((alarmAction == ACTION_OPENDOOR) ? doorOpenGTM.hours() : doorCloseGTM.hours()); DEBUG_PRINT(F("h")); DEBUG_PRINT((alarmAction == ACTION_OPENDOOR) ? doorOpenGTM.minutes() : doorCloseGTM.minutes()); DEBUG_PRINT(F("m to ")); DEBUG_PRINTLN((alarmAction == ACTION_OPENDOOR) ? F("OPEN") : F("CLOSE"));
  doorOpenGTM = TimeSpan(0, now.hour(), now.minute() + 1, 0);
  doorCloseGTM = TimeSpan(0, now.hour(), now.minute() + 1, 0);
  actiondebug = (actiondebug == ACTION_OPENDOOR) ? ACTION_CLOSEDOOR:ACTION_OPENDOOR;
  alarmAction = actiondebug;
#endif

  DEBUG_PRINT(F(" alarm at ")); DEBUG_PRINT((alarmAction == ACTION_OPENDOOR) ? doorOpenGTM.hours() : doorCloseGTM.hours()); DEBUG_PRINT(F("h")); DEBUG_PRINT((alarmAction == ACTION_OPENDOOR) ? doorOpenGTM.minutes() : doorCloseGTM.minutes()); DEBUG_PRINT(F("m to ")); DEBUG_PRINTLN((alarmAction == ACTION_OPENDOOR) ? F("OPEN") : F("CLOSE"));

  DS3231M.setAlarm(minutesHoursMatch, DateTime(0, 0, 0, (alarmAction == ACTION_OPENDOOR) ? doorOpenGTM.hours() : doorCloseGTM.hours(), (alarmAction == ACTION_OPENDOOR) ? doorOpenGTM.minutes() : doorCloseGTM.minutes()));
  attachInterrupt(digitalPinToInterrupt(DS3231M_INTERRUPTION_PIN), onAlarm, LOW );
  delay(100);
}

TimeSpan getSunGTMCalendar(byte index, boolean sunrise) {
  return TimeSpan(0, (sunrise) ? SunRiseGTMHours[index] : SunSetGTMHours[index], (sunrise) ? SunRiseGTMMinutes[index] : SunSetGTMMinutes[index], 0);
}
TimeSpan getSunGTMCalendar(DateTime now, boolean sunrise) {

  // get current 1/2 month index
  byte index = getDateIndex(now.day(), now.month());

  // get end of 1/2 month sun time
  TimeSpan rightEdge = getSunGTMCalendar(index, sunrise);
  // get begin of 1/2 month sun time
  TimeSpan leftEdge = getSunGTMCalendar(((index - 1) < 0) ? 24 : (index - 1), sunrise);

  // compute time span between begin and end of 1/2 month
  int delta = (rightEdge - leftEdge).totalseconds();
  //  DEBUG_PRINTLN(); DEBUG_PRINT(F(" delta:")); DEBUG_PRINT(delta);

  // split it in 15
  float stepTimeSpan = delta / 15.0;
  //  DEBUG_PRINTLN();DEBUG_PRINT(F(" stepTimeSpan:"));DEBUG_PRINT(stepTimeSpan);

  // compute linear evaluation of now
  int offset = (((now.day() > 15) ? now.day() - 15 : now.day()) - 1) * stepTimeSpan;
  //  DEBUG_PRINTLN();DEBUG_PRINT(F(" offset:"));DEBUG_PRINT(offset);

  // compute sunset for today
  TimeSpan sunGtmTime = leftEdge + TimeSpan(offset);
  //  DEBUG_PRINTLN();DEBUG_PRINT(F(" sunGtmTime:"));DEBUG_PRINT(sunGtmTime.hours());DEBUG_PRINT(F("h"));DEBUG_PRINTLN(sunGtmTime.minutes());

  // skip seconds
  return TimeSpan(0,sunGtmTime.hours(),sunGtmTime.minutes(),0);
}
