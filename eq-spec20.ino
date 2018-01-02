#include "Display.h"
#include "GsWebhook.h"
#include "Spec20.h"
#include "PushButton.h"

// 20x4 display, don't reserve anything for messages
Display lcd (0x27, 20, 4, 0);

// google spreadhseet hook
GsWebhook gs ("kopflab_eq");

// spectrophotometer
Spec spec;
bool spec_active = false;

// on/off push button
PushButton button(D5, LOW);
const int LED_PIN = D4;
const int RELAY_PIN = D6;

// relay delay off
bool relay_delay_active = false;
long relay_trigger_time = millis();
const int RELAY_OFF_DELAY = 3000; // 3 second delay

// time sync
#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)
unsigned long last_sync = millis();

// LCD messages
int read_counter = 1;
char read_memory1[21] = "";
char read_memory2[21] = "";
char read_memory3[21] = "";
char read_memory4[21] = "";
char date_time_buffer[21];
char message_buffer[21];

// setup function
void setup(void) {
    // serial for debugging
    Serial.begin(9600);
    Serial.println("INFO: Starting up Spec20 interface");

    // time adjusment
    Time.zone(-6); // to get the time correctly

    // LCD screen
    lcd.init();

    // Google spreadsheets logger
    gs.init();

    // spectrophotometer
    spec.init();

    // on/off button and LED
    button.init();
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
}

// turn spec on
void turn_on () {
    // turn on
    spec_active = true;
    read_counter = 1;

    // reset spec
    spec.reset();

    // turned on message
    Time.format(Time.now(), "%H:%M:%S %m/%d").toCharArray(date_time_buffer, sizeof(date_time_buffer));
    snprintf(message_buffer, sizeof(message_buffer), "ON  @ %s", date_time_buffer);

    digitalWrite(LED_PIN, HIGH);
    digitalWrite(RELAY_PIN, HIGH);
    update_screen(message_buffer);
    relay_delay_active = false;

    // google spreadsheet logging
    gs.send("event", "spec20", "1", "binary", "", "turned ON");
}

// sechedule turn_off
void schedule_turn_off() {
  // don't read spec anymore
  spec_active = false;

  // LED and LCD
  digitalWrite(LED_PIN, LOW);
  snprintf(message_buffer, sizeof(message_buffer), "OFF in %d seconds", RELAY_OFF_DELAY/1000);
  update_screen(message_buffer);

  // schedule relay off
  relay_trigger_time = millis();
  relay_delay_active = true;
}

// turn spec off
void turn_off_spec () {
    // message and relay off
    digitalWrite(RELAY_PIN, LOW);
    Time.format(Time.now(), "%H:%M:%S %m/%d").toCharArray(date_time_buffer, sizeof(date_time_buffer));
    snprintf(message_buffer, sizeof(message_buffer), "OFF @ %s", date_time_buffer);
    update_screen(message_buffer, false);
    relay_delay_active = false;

    // google spreadsheet logging
    gs.send("event", "spec20", "0", "binary", "", "turned OFF");
}

// save reading
bool save_reading () {
    // generate variable
    char variable[20];
    switch (spec.mode[0]) {
          case 'A':
            strcpy(variable, "absorbance");
            break;
          case 'T':
            strcpy(variable, "transmittance");
            break;
          case 'C':
            strcpy(variable, "concentration");
            break;
          case 'F':
            strcpy(variable, "factor");
            break;
    }

    // units
    char units[10];
    snprintf(units, sizeof(units), "%s@%snm", spec.mode, spec.wavelength);

    // save to GS
    char counter[3];
    snprintf(counter, sizeof(counter), "%d", read_counter);
    bool gs_write = gs.send("data", variable, spec.value, units, counter);

    // screen info
    if (gs_write) {
      Time.format(Time.now(), "%M:%S").toCharArray(date_time_buffer, sizeof(date_time_buffer));
      snprintf(message_buffer, sizeof(message_buffer), "#%2d %s%s %s %s",
              read_counter, spec.mode, spec.wavelength, spec.value, date_time_buffer);
    } else {
      snprintf(message_buffer, sizeof(message_buffer), "#%2d %s%s %s no GS",
              read_counter, spec.mode, spec.wavelength, spec.value);
    }
    update_screen(message_buffer);
    spec.reset();
    read_counter++;

    // save successful?
    return(gs_write);
}

// update LCD
void update_screen(const char* message) {
  update_screen(message, true);
}
void update_screen(const char* message, bool push_memory) {
  Serial.println(message);
  if (push_memory) {
    strncpy(read_memory4, read_memory3, sizeof(read_memory3)-1);
    strncpy(read_memory3, read_memory2, sizeof(read_memory3)-1);
    strncpy(read_memory2, read_memory1, sizeof(read_memory2)-1);
  }
  strncpy(read_memory1, message, sizeof(read_memory1)-1);
  lcd.print_line(1, read_memory1);
  if (push_memory) {
    lcd.print_line(2, read_memory2);
    lcd.print_line(3, read_memory3);
    lcd.print_line(4, read_memory4);
  }
}


// loop function
void loop(void) {

    // Time sync (once a day)
    if (millis() - last_sync > ONE_DAY_MILLIS) {
        // Request time synchronization from the Particle Cloud
        Particle.syncTime();
        last_sync = millis();
    }

    // on  / off button
    button.update();
    if (button.was_pushed()) {
      if (spec_active) {
          schedule_turn_off();
      } else {
          turn_on();
      }
    }

    // relay delay
    if (relay_delay_active && (millis() - relay_trigger_time) > RELAY_OFF_DELAY) {
      turn_off_spec();
    }

    // readings
    if (spec_active && spec.check_for_reading()) {
        save_reading();
    }

}
