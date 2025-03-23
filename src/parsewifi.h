
#pragma once


// Struct to hold the parsed WiFi configuration
struct WiFiConfig {
    String SSID;
    String type;
    String password;
};

WiFiConfig parseWiFiQR(const String& qrText);