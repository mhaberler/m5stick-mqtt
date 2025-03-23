#include <Arduino.h>
#include "parsewifi.h"


// Function to unescape special characters
String unescape(const String& str) {
    String result = "";
    int i = 0;
    while (i < str.length()) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            char next = str[i + 1];
            if (next == '\\') {
                result += '\\';
            } else if (next == ';') {
                result += ';';
            } else if (next == ',') {
                result += ',';
            } else if (next == '"') {
                result += '"';
            } else if (next == ':') {
                result += ':';
            } else {
                // Unknown escape, add both
                result += '\\';
                result += next;
            }
            i += 2; // Skip both \\ and next
        } else {
            result += str[i];
            i++;
        }
    }
    return result;
}

// Helper function to process a single key-value pair
void processPair(const String& pair, WiFiConfig& config) {
    int colon = pair.indexOf(':');
    if (colon != -1) {
        String key = pair.substring(0, colon);
        String value = pair.substring(colon + 1);
        value = unescape(value);
        if (value.startsWith("\"") && value.endsWith("\"")) {
            value = value.substring(1, value.length() - 1);
        }
        if (key == "S") {
            config.SSID = value;
        } else if (key == "T") {
            config.type = value;
        } else if (key == "P") {
            config.password = value;
        }
        // Add more fields if needed (e.g., H for hidden networks)
    }
}

// Main function to parse WiFi QR code text
WiFiConfig parseWiFiQR(const String& qrText) {
    WiFiConfig config;
    if (!qrText.startsWith("WIFI:")) {
        // Handle error: not a valid WiFi QR code
        return config;
    }
    String content = qrText.substring(5); // Remove "WIFI:"
    // Remove trailing semicolons
    while (content.endsWith(";")) {
        content = content.substring(0, content.length() - 1);
    }
    // Split by semicolons
    int start = 0;
    int end = content.indexOf(';');
    while (end != -1) {
        String pair = content.substring(start, end);
        processPair(pair, config);
        start = end + 1;
        end = content.indexOf(';', start);
    }
    // Process the last pair
    String lastPair = content.substring(start);
    processPair(lastPair, config);
    return config;
}


// for(int i = 0; i < width * height; i++) {
//     image[i] = fb->buf[2 * i]; // Y channel
// }

// .pixel_format  = PIXFORMAT_RGB565,

// #include <cstdint>

// uint8_t extractLuminanceInt(uint16_t pixel) {
//     uint8_t R = (pixel >> 11) & 0x1F; // 0-31
//     uint8_t G = (pixel >> 5) & 0x3F;  // 0-63
//     uint8_t B = pixel & 0x1F;         // 0-31

//     // BT.709 coefficients scaled by 1024: 0.2126*1024=218, 0.7152*1024=732, 0.0722*1024=74
//     uint32_t luminance = (218 * R + 732 * G + 74 * B);

//     // Normalize to 0-255: max value is (218*31 + 732*63 + 74*31) = 55220, so divide by 216 (>> 8 approx)
//     return (luminance >> 8); // Approximate division by 256
// }