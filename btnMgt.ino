byte btnAction = ACTION_OPENDOOR;

void setupBtn() {
  /* Setup the pin direction. */
  pinMode(3, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(3), onBtn, LOW );
}

void onBtn() {
  cli();
  detachInterrupt(digitalPinToInterrupt(3));
  sleep_disable();
  state = STATE_BTNACTION;
  btnAction = (btnAction == ACTION_OPENDOOR) ? ACTION_CLOSEDOOR : ACTION_OPENDOOR;
  sei();
}

void performBtnAction() {
  if ( digitalRead(3) == LOW) {
    moveManuDoor(btnAction == ACTION_OPENDOOR);
  } else {
    DEBUG_PRINTLN(F(""));
    DEBUG_PRINTLN(F("stopManuDoor"));
    stopManuDoor();
    delay(10);
    attachInterrupt(digitalPinToInterrupt(3), onBtn, LOW );
    state = STATE_IDLE;
  }
}
