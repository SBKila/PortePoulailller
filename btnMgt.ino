
#define BTN_PIN 3

void setupBtn() {
  DEBUG_PRINTLN(F("setup Btn"));
  /* Setup the pin direction. */
  pinMode(BTN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), onBtn, LOW );
}
boolean isBtnPushed(){
  return (digitalRead(BTN_PIN) == LOW);
}

void onBtn() {
  cli();
  detachInterrupt(digitalPinToInterrupt(BTN_PIN));
  sleep_disable();
  action = ACTION_BTN;
  sei();
}
