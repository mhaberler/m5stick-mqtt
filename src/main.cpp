#include <M5Unified.h>
#include <WiFi.h>
#include <PicoMQTT.h>
#include <PicoWebsocket.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <WebServer.h>


WiFiServer tcp_server(1883);
WiFiServer websocket_underlying_server(81);
PicoWebsocket::Server<::WiFiServer> websocket_server(websocket_underlying_server);
PicoMQTT::Server mqtt(tcp_server, websocket_server);

WebServer server(80);

wl_status_t wifi_status = WL_STOPPED;
const int RED_LED_PIN = 10;  // Red LED on M5Stick-CPlus

const char* hostname = "picomqtt";



void handleNotFound() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
    log_i("reply: %s", message.c_str());
}

void setup() {
    auto cfg = M5.config();
    cfg.serial_baudrate = 115200;
    M5.begin(cfg);

    // Rotate screen if needed (adjust based on your orientation)
    M5.Display.setRotation(1);
    M5.Display.setTextSize(2);

    // Initialize IMU
    M5.Imu.init();

#ifdef ARDUINO_M5Stick_C
    // Initialize LED pin
    pinMode(RED_LED_PIN, OUTPUT);
    digitalWrite(RED_LED_PIN, HIGH);  // HIGH is off for this LED
#endif

    // Start WiFi in background
    WiFi.setHostname(hostname);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    // Start mDNS
    if (MDNS.begin(hostname)) {
        MDNS.addService("http", "tcp", 80);
        MDNS.addService("mqtt", "tcp", 1883);
        MDNS.addService("mqtt-ws", "tcp", 81);
        mdns_service_instance_name_set("_mqtt", "_tcp", "MQTT/TCP broker");
        mdns_service_instance_name_set("_mqtt-ws", "_tcp", "MQTT/Websockets broker");
        mdns_service_instance_name_set("_http", "_tcp", "HTTP Push server");
    }
    mqtt.begin();
    mqtt.subscribe("expo/message", [](const char * payload) {
        if (payload && strlen(payload)) {
            M5.Display.println(payload);
            Serial.printf("Received message in topic 'expo/message': %s\n", payload);
        }
    });

    server.on("/", []() {
        server.send(200, "text/plain", "hello from esp32!");
    });
    server.onNotFound(handleNotFound);

}

void loop() {
    M5.update();

    wl_status_t ws = WiFi.status();
    if (ws ^ wifi_status) {
        wifi_status = ws; // track changes

        M5.Display.clear();
        M5.Display.setCursor(0, 0);
        switch (ws) {
            case WL_CONNECTED:
                M5.Display.println("WiFi: Connected");
                M5.Display.print("IP: ");
                M5.Display.println(WiFi.localIP());

                server.begin();
                break;
            case WL_NO_SSID_AVAIL:
                M5.Display.printf("WiFi: SSID\n%s\nnot found\n", WIFI_SSID);
                break;
            case WL_DISCONNECTED:
                M5.Display.printf("WiFi: disconnected\n");
                break;
            default:
                M5.Display.printf("WiFi status: %d\n", ws);
                break;
        }
        log_i("wifi_status=%d", wifi_status);
    }

    if (wifi_status == WL_CONNECTED) {
        // Publish temperature every 5 seconds
        static unsigned long lastPublish = 0;
        if (millis() - lastPublish >= 1000) {

#ifdef ARDUINO_M5Stick_C
            // Blink LED - turn on (LOW because LED is active-low)
            digitalWrite(RED_LED_PIN, LOW);
#endif

            JsonDocument doc;

            // Get battery info
            doc["battery_voltage"] = M5.Power.getBatteryVoltage() / 1000.0;  // Convert mV to V
            doc["battery_level"] = M5.Power.getBatteryLevel();  // 0-100%

            // Get IMU data
            float accX, accY, accZ;
            float gyroX, gyroY, gyroZ;
            M5.Imu.getAccel(&accX, &accY, &accZ);
            M5.Imu.getGyro(&gyroX, &gyroY, &gyroZ);

            JsonObject imu = doc["imu"].to<JsonObject>();
            imu["accel_x"] = accX;
            imu["accel_y"] = accY;
            imu["accel_z"] = accZ;
            imu["gyro_x"] = gyroX;
            imu["gyro_y"] = gyroY;
            imu["gyro_z"] = gyroZ;

            // Serialize to string and publish
            String jsonString;
            serializeJson(doc, jsonString);
            mqtt.publish("sensors", jsonString);
            lastPublish = millis();

            // Brief delay for visible blink (100ms)
            delay(100);
#ifdef ARDUINO_M5Stick_C

            // Turn LED off
            digitalWrite(RED_LED_PIN, HIGH);
#endif
        }

        if (M5.BtnB.isPressed()) {
            ESP.restart();
        }
        // Button state check
        static bool lastButtonState = false;

        bool currentButtonState = M5.BtnA.isPressed();
        if (currentButtonState != lastButtonState) {
            if (wifi_status == WL_CONNECTED) {
#ifdef ARDUINO_M5Stick_C
                // Blink LED - turn on (LOW because LED is active-low)
                digitalWrite(RED_LED_PIN, LOW);
#endif
                JsonDocument doc;
                doc["button"] = currentButtonState;
                log_i("button: %u", currentButtonState);
                String jsonString;
                serializeJson(doc, jsonString);
                mqtt.publish("button/state", jsonString);
#ifdef ARDUINO_M5Stick_C
                // Turn LED off
                digitalWrite(RED_LED_PIN, HIGH);
#endif
            }
            lastButtonState = currentButtonState;
        }
        // Handle MQTT
        mqtt.loop();
    }
    server.handleClient();

    yield();
}