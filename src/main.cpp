/*
  M5Stack CoreS3 Camera Capture Example
  This sketch uses M5Unified and M5GFX to capture an image from the built-in camera
  and display it on the screen when a button is pressed.
*/

#include <M5Unified.h>
#include <M5GFX.h>

// Camera settings
#define CAMERA_MODEL_M5STACK_CORES3
#define CAMERA_PIN_PWDN -1
#define CAMERA_PIN_RESET -1
#define CAMERA_PIN_XCLK 45
#define CAMERA_PIN_SIOD 39
#define CAMERA_PIN_SIOC 40
#define CAMERA_PIN_D7 48
#define CAMERA_PIN_D6 11
#define CAMERA_PIN_D5 12
#define CAMERA_PIN_D4 14
#define CAMERA_PIN_D3 16
#define CAMERA_PIN_D2 18
#define CAMERA_PIN_D1 17
#define CAMERA_PIN_D0 15
#define CAMERA_PIN_VSYNC 38
#define CAMERA_PIN_HREF 47
#define CAMERA_PIN_PCLK 13

#include "esp_camera.h"

static camera_config_t camera_config = {
  .pin_pwdn = CAMERA_PIN_PWDN,
  .pin_reset = CAMERA_PIN_RESET,
  .pin_xclk = CAMERA_PIN_XCLK,
  .pin_sccb_sda = CAMERA_PIN_SIOD,
  .pin_sccb_scl = CAMERA_PIN_SIOC,
  .pin_d7 = CAMERA_PIN_D7,
  .pin_d6 = CAMERA_PIN_D6,
  .pin_d5 = CAMERA_PIN_D5,
  .pin_d4 = CAMERA_PIN_D4,
  .pin_d3 = CAMERA_PIN_D3,
  .pin_d2 = CAMERA_PIN_D2,
  .pin_d1 = CAMERA_PIN_D1,
  .pin_d0 = CAMERA_PIN_D0,
  .pin_vsync = CAMERA_PIN_VSYNC,
  .pin_href = CAMERA_PIN_HREF,
  .pin_pclk = CAMERA_PIN_PCLK,
  
  // XCLK 20MHz or 10MHz
  .xclk_freq_hz = 20000000,
  .ledc_timer = LEDC_TIMER_0,
  .ledc_channel = LEDC_CHANNEL_0,
  
  .pixel_format = PIXFORMAT_RGB565, // For display: RGB565
  .frame_size = FRAMESIZE_QVGA,     // QVGA = 320x240
  
  .jpeg_quality = 12,               // 0-63, lower means higher quality
  .fb_count = 2,                    // Number of frame buffers to be allocated
  .grab_mode = CAMERA_GRAB_WHEN_EMPTY
};

bool setupCamera() {
  // Initialize the camera
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK) {
    M5.Display.printf("Camera init failed with error 0x%x", err);
    return false;
  }
  return true;
}

void captureAndDisplayImage() {
  // Capture frame
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    M5.Display.println("Camera capture failed");
    return;
  }
  
  // Display image if it's RGB565 format (direct display)
  if (fb->format == PIXFORMAT_RGB565) {
    M5.Display.setAddrWindow(0, 0, fb->width, fb->height);
    M5.Display.pushColors((uint16_t*)fb->buf, fb->len / 2);
  } else {
    M5.Display.println("Image format not supported for direct display");
  }
  
  // Return the frame buffer to the camera
  esp_camera_fb_return(fb);
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  
  M5.Display.setRotation(1); // Landscape orientation
  M5.Display.setTextSize(2);
  M5.Display.println("M5Stack CoreS3 Camera Example");
  M5.Display.println("Press button A to capture");
  
  if (!setupCamera()) {
    M5.Display.println("Failed to initialize camera!");
    while (1) {
      delay(100);
    }
  }
  
  M5.Display.println("Camera initialized successfully!");
  delay(1000);
}

void loop() {
  M5.update();  // Update button state
  
  if (M5.BtnA.wasPressed()) {
    M5.Display.clear();
    M5.Display.setCursor(0, 0);
    M5.Display.println("Capturing image...");
    delay(500); // Small delay before capture
    
    captureAndDisplayImage();
    
    M5.Display.setCursor(0, M5.Display.height() - 30);
    M5.Display.println("Press A to capture again");
  }
  
  delay(10);
}