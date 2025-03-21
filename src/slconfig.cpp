
#include "slconfig.hpp"
#include "base64.h"

extern "C" {
#include  "zstd.h"
}

String prefix = "https://sensorlogger.app/link/config/";

// take a string, compress and b64encode, prefix with sensorlogger deeplink
bool sensorloggerCfg(const String &input, String &output) {
    // determine max compressed  size
    size_t buffer_size = ZSTD_compressBound(input.length());
    uint8_t *buffer = (uint8_t *)malloc(buffer_size);
    if (buffer == NULL)
        return false;

    // Compress
    size_t compressed_size = ZSTD_compress(buffer, buffer_size, input.c_str(), input.length(), 0);

    if (ZSTD_isError(compressed_size)) {
        log_e("compression error: %s\n", ZSTD_getErrorName(compressed_size));
        free(buffer);
        return false;
    }
    output = prefix + base64::encode(buffer, compressed_size) + "==";

    log_i("src %u compressed %u final %zu",input.length(), compressed_size, output.length());
    free(buffer);
    return true;
}

// take a JsonDocument, compress and b64encode, prefix with sensorlogger deeplink
bool sensorloggerCfg(const JsonDocument &doc, String &output) {
    String jsonString;
    serializeJson(doc, jsonString);
    return sensorloggerCfg(jsonString, output);
}

bool genLoggingCfg(const String &ip,
                   uint16_t mqtt_port,
                   const String &http_uri,
                   const String &subscribe_topic,
                   String &result
                  ) {
    JsonDocument doc;
    if (http_uri.length()) {
        JsonObject http = doc["http"].to<JsonObject>();
        http["enabled"] = false;
        http["url"] = http_uri;
        http["batchPeriod"] = 1000;
        http["skip"] = false;
    }
    if (mqtt_port) {
        JsonObject mqtt = doc["mqtt"].to<JsonObject>();
        mqtt["enabled"] = false;
        mqtt["url"] = ip;
        mqtt["port"] = mqtt_port;
        mqtt["tls"] = false;
        mqtt["topic"] = "sensor-logger";
        mqtt["batchPeriod"] = 5000;
        mqtt["connectionType"] = "TCP";
        mqtt["subscribeTopic"] = subscribe_topic;
        mqtt["skip"] = false;
        mqtt["subscribeEnabled"] = true;
    }
    return sensorloggerCfg(doc, result);
}


// recreated from sensorlogger "reset to defaults" and exported
void genDefaultCfg(JsonDocument &doc) {

    // Create sensorState object
    JsonObject sensorState = doc["sensorState"].to<JsonObject>();

    // Add sensor objects with their properties
    sensorState["Orientation"]["enabled"] = false;
    sensorState["Magnetometer"]["enabled"] = false;
    sensorState["Compass"]["enabled"] = false;
    sensorState["Barometer"]["enabled"] = false;
    sensorState["Location"]["enabled"] = false;
    sensorState["Accelerometer"]["enabled"] = false;

    sensorState["Gravity"].to<JsonObject>();
    sensorState["Gyroscope"].to<JsonObject>();

    sensorState["Microphone"]["enabled"] = false;
    sensorState["Bluetooth"]["enabled"] = false;

    sensorState["uncalibrated"] = false;

    // Create http object
    JsonObject http = doc["http"].to<JsonObject>();
    http["enabled"] = true;
    http["url"] = "http://192.168.1.99:8000/data";
    http["batchPeriod"] = 1000;

    // Create mqtt object
    JsonObject mqtt = doc["mqtt"].to<JsonObject>();
    mqtt["enabled"] = true;
    mqtt["url"] = "192.168.1.99";
    mqtt["port"] = "8884";
    mqtt["tls"] = true;
    mqtt["topic"] = "sensor-logger";
    mqtt["batchPeriod"] = 1000;
    mqtt["connectionType"] = "Websocket";
    mqtt["subscribeTopic"] = "";
    mqtt["skip"] = true;
    mqtt["subscribeEnabled"] = true;
}
