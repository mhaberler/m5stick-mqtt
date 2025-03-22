#include <M5CoreS3.h>
#include <quirc.h>
#include "esp_camera.h"

void setup() {
    M5.begin();
    auto cfg = M5.config();
    CoreS3.begin(cfg);
    CoreS3.Display.setTextColor(GREEN);
    CoreS3.Display.setTextDatum(middle_center);
    CoreS3.Display.setFont(&fonts::Orbitron_Light_24);
    CoreS3.Display.setTextSize(1);

    if (!CoreS3.Camera.begin()) {
        CoreS3.Display.drawString("Camera Init Fail", CoreS3.Display.width() / 2, CoreS3.Display.height() / 2);
    }
    CoreS3.Display.drawString("Camera Init Success", CoreS3.Display.width() / 2, CoreS3.Display.height() / 2);
}

void loop() {
    // Assume camera is set to YUV422
    if (CoreS3.Camera.get()) {
        camera_fb_t *fb = CoreS3.Camera.fb;
        log_i("got fb %p", fb);

        if (fb) {
#if 0
            int width = fb->width;
            int height = fb->height;
            struct quirc *qr = quirc_new();
            if (qr && quirc_resize(qr, width, height) >= 0) {
                uint8_t *image = quirc_begin(qr, &width, &height);
                if (image) {
                    for(int i = 0; i < width * height; i++) {
                        image[i] = fb->buf[2 * i]; // Y channel
                    }
                    quirc_end(qr);
                    int num_codes = quirc_count(qr);
                    for (int i = 0; i < num_codes; i++) {
                        struct quirc_code code;
                        struct quirc_data data;
                        quirc_extract(qr, i, &code);
                        quirc_decode_error_t err = quirc_decode(&code, &data);
                        if (!err) {
                            String payload = String((const char *)data.payload);
                            CoreS3.Display.clear();
                            String text = "QR Code: " + payload;
                            CoreS3.Display.drawString(text.c_str(), 0, 0);
                            delay(3000);
                        }
                    }
                }
                quirc_destroy(qr);
            }
#endif
            CoreS3.Camera.free();
            // esp_camera_fb_return(fb);
        }
    } else {
        log_e("CoreS3.Camera.get() fail");
        delay(1000);
    }
    yield();
}