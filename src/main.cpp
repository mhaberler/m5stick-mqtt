#include <M5Unified.h>

void setup() {
  // Initialize M5Stack CoreS3 hardware, including display, buttons, and SD card
  M5.begin();

  // Configure the camera to output JPEG images at SVGA resolution (800x600)
  M5.Camera.setPixelFormat(PIXFORMAT_JPEG);
  M5.Camera.setFrameSize(FRAMESIZE_SVGA);

  // Initialize the camera
  M5.Camera.begin();

  // Set up the display for text output
  M5.Display.setTextSize(2);       // Increase text size for readability
  M5.Display.setTextColor(WHITE);  // Set text color to white
  M5.Display.setCursor(0, 0);      // Position cursor at top-left
  M5.Display.println("Press A to capture image");  // Display initial instruction
}

void loop() {
  // Update button states
  M5.update();

  // Check if Button A was pressed
  if (M5.BtnA.wasPressed()) {
    // Capture a frame from the camera
    auto frame = M5.Camera.getFrame();
    
    if (frame) {  // Ensure frame was captured successfully
      // Open a file on the SD card in write mode
      File file = SD.open("/image.jpg", FILE_WRITE);
      
      if (file) {  // Ensure file opened successfully
        // Write the frame buffer to the file
        file.write(frame->buf, frame->len);
        file.close();  // Close the file

        // Display confirmation message
        M5.Display.clear();           // Clear the screen
        M5.Display.setCursor(0, 0);   // Reset cursor position
        M5.Display.println("Image saved as /image.jpg");
        delay(2000);                  // Show message for 2 seconds

        // Restore initial instruction
        M5.Display.clear();
        M5.Display.setCursor(0, 0);
        M5.Display.println("Press A to capture image");
      } else {
        // Display error if file couldn't be opened
        M5.Display.println("Failed to open file");
      }

      // Release the frame buffer
      M5.Camera.returnFrame(frame);
    } else {
      // Display error if frame capture failed
      M5.Display.println("Failed to capture image");
    }
  }
}