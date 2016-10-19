/*
 * PushButton.h
 * Created on: October 19, 2016
 * Author: Sebastian Kopf <sebastian.kopf@colorado.edu>
 * Debouncing a push button to trigger an event
 * Usage:

 *
 */

 #pragma once
 #include "application.h"

 class PushButton {

   uint8_t pin;
   bool active_mode;

   bool stable_state;
   bool last_stable_state;

   bool debounce_state;
   long last_debounce_time;

 public:

   // constructor
   PushButton(uint8_t pin, bool active_mode) : pin(pin), active_mode(active_mode) {
      debounce_delay = 20; // 20 ms debounce delay
      debounce_state = !active_mode;
      stable_state = !active_mode;
      last_stable_state = !active_mode;
   }

   long debounce_delay;
   void init(); // initialize button
   bool is_active (); // whether the button is currently active or not
   bool was_pushed(); // whether the button was just pushed
   bool was_released(); // whether the button was just released
   void update(); // update the state of the button
};

void PushButton::init() {
  // pullup resistor depending on whether button is high or low
  if (active_mode == HIGH) {
    pinMode(pin, INPUT_PULLDOWN);
  } else {
    pinMode(pin, INPUT_PULLUP);
  }
  last_debounce_time = millis();
}

void PushButton::update() {

  // read the state of the button
  int reading = digitalRead(pin);

  // if state changed, due to noise or pressing:
  if (reading != debounce_state) {
    // reset the debouncing timer
    last_debounce_time = millis();
    debounce_state = reading;
  }

  // update states if it's been long enough to count as a real signal
  if ((millis() - last_debounce_time) > debounce_delay) {
    last_stable_state = stable_state;
    stable_state = debounce_state;
  }
}

bool PushButton::is_active() {
  if (stable_state == active_mode) {
    return(true);
  } else {
    return(false);
  }
}

bool PushButton::was_pushed() {
  if (last_stable_state != stable_state && is_active()) {
    return(true);
  } else {
    return(false);
  }
}

bool PushButton::was_released() {
  if (last_stable_state != stable_state && !is_active()) {
    return(true);
  } else {
    return(false);
  }
}
