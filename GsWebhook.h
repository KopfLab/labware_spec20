/*
 * GsWebhook.h
 * Created on: Marc 3, 2016
 * Author: Sebastian Kopf <seb.kopf@gmail.com>
 * Convenience wrapper for logging to google spreadsheet via a webhook
 * Usage:

GsWebhook gs ("webhook_name"); // insert name of GS logging web hook (can only be 12 chars long!!!)
void setup () {
    gs.init();
    // send to google spread sheet (value can be char/String/int/double, unit, sample and message are optional)
    gs.send("data type", "variable name", "value", "unit", "sample", "msg");
}

*/

#pragma once
#include "application.h"

class GsWebhook {
  // config (web vars can only be 12 chars!)
  char webhook[12];

  // data
  char date_time_buffer[20];
  char value_buffer[200];
  char json_buffer[255];

public:
  // constructor
  GsWebhook(const char* webhook) {
    //if (strlen(webhook) > sizeof(this->webhook)) // throw some kind of exceptoin!
    strcpy( this->webhook, webhook );
  };

  // methods
  bool init ();

  // send character value
  bool send (const char* type, const char* variable, const char* value,
    const char* unit = "", const char* sample = "", const char* msg = "") ;

  // send String value
  bool send (const char* type, const char* variable, const String &value,
    const char* unit = "", const char* sample = "", const char* msg = "") ;

  // send integer value
  bool send (const char* type, const char* variable, int value,
    const char* unit = "", const char* sample = "", const char* msg = "") ;

  // send double value
  bool send (const char* type, const char* variable, double value,
    const char* unit = "", const char* sample = "", const char* msg = "") ;
};

bool GsWebhook::init() {
  return(send("event", "startup", "", "", "", "complete"));
}

bool GsWebhook::send(const char* type, const char* variable, const char* value,
  const char* unit, const char* sample, const char* msg) {
  Time.format(Time.now(), "%Y-%m-%d %H:%M:%S").toCharArray(date_time_buffer, sizeof(date_time_buffer));
  snprintf(json_buffer, sizeof(json_buffer),
    "{\"datetime\":\"%s\",\"type\":\"%s\",\"var\":\"%s\",\"value\":\"%s\", \"units\":\"%s\", \"sample\":\"%s\",\"msg\":\"%s\"}",
    date_time_buffer, type, variable, value, unit, sample, msg);
  return(Particle.publish(webhook, json_buffer));
}

bool GsWebhook::send(const char* type, const char* variable, const String &value,
  const char* unit, const char* sample, const char* msg) {
  value.toCharArray(value_buffer, sizeof(value_buffer));
  return(send(type, variable, value_buffer, unit, msg, sample));
}

bool GsWebhook::send(const char* type, const char* variable, int value,
  const char* unit, const char* sample, const char* msg) {
  sprintf(value_buffer, "%d", value);
  return(send(type, variable, value_buffer, unit, msg, sample));
}

bool GsWebhook::send(const char* type, const char* variable, double value,
  const char* unit, const char* sample, const char* msg) {
  sprintf(value_buffer, "%.3f", value);
  return(send(type, variable, value_buffer, unit, msg, sample));
}
