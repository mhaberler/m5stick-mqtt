#pragma once

#include <ArduinoJson.h>

bool sensorloggerCfg(const String &input, String &output);
bool sensorloggerCfg(const JsonDocument &doc, String &output);
bool genLoggingCfg(const String &ip,
                   uint16_t mqtt_port,
                   const String &http_uri,
                   const String &subscribe_topic,
                   String &result
                  );

void genDefaultCfg(JsonDocument &doc);