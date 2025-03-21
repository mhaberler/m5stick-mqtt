#pragma once

#include <ArduinoJson.h>

bool sensorloggerCfg(const String &input, String &output);
bool sensorloggerCfg(const JsonDocument &doc, String &output);
void genDefaultCfg(JsonDocument &doc);