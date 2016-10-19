/*
 * Spec20.h
 * Created on: October 10, 2016
 * Author: Sebastian Kopf <sebastian.kopf@colorado.edu>
 * Convenience wrapper for receiving serial output from a spectronic 20 photospectrometer on a particle photon
 * Usage:

Spec20 spec ();
void setup () {
  spec.init();
}

void loop() {
  if (spec.check_for_reading()) {
      // some action accessing spec.wavelength, .value and .mode
      spec.reset();
  }
}
*/

#pragma once
#include "application.h"

class Spec {

    String temp_reading = ""; // format: 630NM -0.15 A
    int position; // message position
    bool ready_to_receive; // whether ready to read next value

  public:

    char wavelength[4];
    char value[6];
    char mode[2];

    // constructor
    Spec () {};

    // methods
    void init (); // initialize

    void reset(); // reset the spec (required after each successful reading to get ready for the next message)

    bool check_for_reading(); // check for reading (returns true if new reading is processed)

};


void Spec::init() {
    // serial connection to spec20 on photon
    Serial1.begin(1200);
    reset();
}

void Spec::reset() {
    // reset all variables
    strcpy(wavelength, "");
    strcpy(value, "");
    strcpy(mode, "");
    temp_reading = "";
    ready_to_receive = true;
    position = 0;

    // empty serial read buffer
    while (Serial1.available()) {
      Serial1.read();
    }
}

// spec20 message pattern
const int WL = 1;
const int NM = 2;
const int SIGN = 3;
const int VAL = 4;
const int TYPE = 5;
const int SPACE = 32; // actual ASCII value
const int CR = 13; // actual ASCII value
const int LF = 10; // actual ASCII value
const int PATTERN[16] = {WL, WL, WL, NM, NM, SPACE, SPACE, VAL, VAL, VAL, VAL, VAL, SPACE, TYPE, CR, LF};

bool Spec::check_for_reading() {

  bool new_reading = false;

  // read serial information (if spec is ready to receive)
  while (ready_to_receive && Serial1.available()) {
    byte b = Serial1.read();
    char c = (char) b;


    if (PATTERN[position] == WL && b >= 48 && b<= 57) {
      // wavelength needs to be a number
      temp_reading += c;
      position++;
    } else if (PATTERN[position] == NM && (b == 77 || b == 78) ) {
      temp_reading += c;
      position++;
    } else if (PATTERN[position] == SPACE && b == SPACE) {
      temp_reading += c;
      position++;
    } else if (PATTERN[position] == VAL && ( (b >= 48 && b<= 57) || b == 46 || b == 43 || b == 45 || b == SPACE) ) {
      // +, -, ., space or 0-9
      temp_reading += c;
      position++;
    } else if (PATTERN[position] == TYPE && (c == 'A' || c == 'T' || c == 'C' || c == 'F')) {
      // the reading types
      temp_reading += c;
      position++;
    } else if (PATTERN[position] == CR && b == CR) {
      // carriage return at end of message
      position++;
    } else if (PATTERN[position] == LF && b == LF) {
      // end of complete message
      Serial.print("INFO - reading complete: ");
      Serial.println(temp_reading);

      // fill the reading fields
      char buffer[16];
      temp_reading.toCharArray(buffer, sizeof(buffer));
      memcpy(wavelength, buffer, 3);
      wavelength[3] = 0;
      memcpy(value, buffer + 7, 5);
      value[5] = 0;
      memcpy(mode, buffer + 13, 1);
      mode[1] = 0;

      position = 0;
      new_reading = true;
      ready_to_receive = false;
    } else {
      Serial.print("WARNING - unexpected character triggered message reset: ");
      Serial.print(b);
      Serial.print(" = ");
      Serial.println(c);
      Serial.print("          partial message to this point: ");
      Serial.println(temp_reading);
      reset();
    }

  }

  return(new_reading);
}
