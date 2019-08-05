DS3231M_Class DS3231M;

//Heure
////////////////////////// JANV    FEV    Mars   Avr    Mai    Jun    Juil   Aout   Sept    Oct    Nov    Dec
int LeveSoleilHeure[24] =  { 8, 8,   8, 7,   7, 7,   7, 7,   6, 6,   6, 6,   7, 7,   7, 7,     7, 7,     8, 8,   7, 8,   8, 8  };
int LeveSoleilMinute[24] = { 30, 20, 10, 50, 25, 00, 28, 02, 40, 23, 13, 13, 00, 00, 00, 00,  27, 44,  03, 21, 43, 03, 20, 31  };
int CoucheSoleilHeure[24] = {18, 18, 18, 19, 19, 19, 21, 21, 21, 21, 22, 22, 22, 21, 21, 21,  21, 21,  20, 20, 18, 17, 17, 18  };
int CoucheSoleilMinute[24] = {00, 20, 45, 04, 26, 42, 03, 20, 39, 56, 10, 26, 00, 30, 00, 00,  00, 00,  00, 00, 05, 54, 48, 01  };

byte alarmAction;

void setupRTC() {

  while (!DS3231M.begin()) {                                                  // Initialize RTC communications    //
    DEBUG_PRINTLN(F("Unable to find DS3231MM. Checking again in 3s."));      // Show error text                  //
    delay(3000);                                                              // wait a second                    //
  } // of loop until device is located
  DEBUG_PRINTLN(F("DS3231M initialized."));                                  //                                  //

  //DS3231M.adjust();                                                           // Set to library compile Date/Time //
  DEBUG_PRINT(F("Date/Time set to compile time: "));                         //                                  //

  DateTime now = DS3231M.now();
  DEBUG_PRINT(now.year());
  DEBUG_PRINT(F("-"));
  DEBUG_PRINT(now.month());
  DEBUG_PRINT(F("-"));
  DEBUG_PRINT(now.day());
  DEBUG_PRINT(F(""));
  DEBUG_PRINT(now.hour());
  DEBUG_PRINT(F(":"));
  DEBUG_PRINT(now.minute());
  DEBUG_PRINT(F("."));
  DEBUG_PRINTLN(now.second());
  DEBUG_PRINT(F("DS3231M chip temperature is "));                            //                                  //
  DEBUG_PRINT_NUMBER(DS3231M.temperature() / 100.0, 1);                             // Value is in 100ths of a degree   //
  DEBUG_PRINTLN("\xC2\xB0""C");
  DS3231M.pinSquareWave();
}

void setupRTCAlarm() {
  pinMode(2, INPUT_PULLUP);
}

void onAlarm(void) {
  cli();
  detachInterrupt(digitalPinToInterrupt(2));
  sleep_disable();
  state = STATE_ALARMACTION;
  sei();
}

void setupNextAlarm() {
  DEBUG_PRINTLN(F("setupNextAlarm"));

  DateTime now = DS3231M.now();
  DEBUG_PRINT(F("it is "));
  DEBUG_PRINT(now.hour());
  DEBUG_PRINTLN(F("h"));
  DEBUG_PRINT(now.minute());
  DEBUG_PRINTLN(F("m"));

  byte index = getDateIndex(now.day(), now.month());

  DEBUG_PRINT(F("Open Door at "));
  DEBUG_PRINT(LeveSoleilHeure[index]);
  DEBUG_PRINT(F("h"));
  DEBUG_PRINT(LeveSoleilMinute[index]);
  DEBUG_PRINTLN(F("m"));
  DEBUG_PRINT(F("Close Door at "));
  DEBUG_PRINT(CoucheSoleilHeure[index]);
  DEBUG_PRINT(F("h"));
  DEBUG_PRINT(CoucheSoleilMinute[index]);
  DEBUG_PRINTLN(F("m"));

  byte alarmH;
  byte alarmM;

  if (isBefore(LeveSoleilHeure[index], LeveSoleilMinute[index], now)) {
    alarmH = LeveSoleilHeure[index];
    alarmM = LeveSoleilMinute[index];
    alarmAction = ACTION_OPENDOOR;
  } else if (isBefore(CoucheSoleilHeure[index], CoucheSoleilMinute[index], now)) {
    alarmH = CoucheSoleilHeure[index];
    alarmM = CoucheSoleilMinute[index];
    alarmAction = ACTION_CLOSEDOOR;
  } else {
    alarmH = LeveSoleilHeure[index];
    alarmM = LeveSoleilMinute[index];
    alarmAction = ACTION_OPENDOOR;
  }

  /*** for debug ***/
  //  DateTime alarm = now + TimeSpan(0, 0, 1, 0);
  //  alarmH = alarm.hour();
  //  alarmM = alarm.minute();
  /*** #for debug ***/

  DEBUG_PRINT(F("Alarm at "));
  DEBUG_PRINT(alarmH);
  DEBUG_PRINT(F("h"));
  DEBUG_PRINT(alarmM);
  DEBUG_PRINTLN(F("m"));
  DS3231M.setAlarm(minutesHoursMatch, DateTime(0, 0, 0, alarmH, alarmM));
  attachInterrupt(digitalPinToInterrupt(2), onAlarm, LOW );
  delay(100);
  state = STATE_IDLE;
}

bool isBefore(byte hour, byte minute, DateTime now) {
  return ((now.hour() < hour) || ((now.hour() == hour) && (now.minute() < minute)));
}

byte getDateIndex(byte day, byte month) {
  return (month - 1) * 2 + (day > 15 ? 1 : 0);
}
