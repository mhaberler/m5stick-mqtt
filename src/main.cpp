#include <M5CoreS3.h>
#include <Camera.h>
#include <quirc.h>
// #include "esp_camera.h"

Camera camera;

struct quirc_code *code;
struct quirc_data *data;
void setup() {

    M5.begin();
    auto cfg = M5.config();
    CoreS3.begin(cfg);
    CoreS3.Display.setTextColor(GREEN);
    CoreS3.Display.setTextDatum(middle_center);
    CoreS3.Display.setFont(&fonts::Orbitron_Light_24);
    CoreS3.Display.setTextSize(1);

    if (!camera.begin()) {
        CoreS3.Display.drawString("Camera Init Fail", CoreS3.Display.width() / 2, CoreS3.Display.height() / 2);
    }
    CoreS3.Display.drawString("Camera Init Success", CoreS3.Display.width() / 2, CoreS3.Display.height() / 2);

    code = (struct quirc_code *)ps_malloc(sizeof(struct quirc_code));
    data = (struct quirc_data *)ps_malloc(sizeof(struct quirc_data));

    assert(code != NULL);
    assert(data != NULL);

    delay(3000);
}

void loop() {
    // Assume camera is set to YUV422
    if (camera.get()) {
        camera_fb_t *fb = camera.fb;
        // log_i("got fb %p", fb);

        if (fb) {
            CoreS3.Display.pushImage(0, 0, CoreS3.Display.width(), CoreS3.Display.height(),
                                     (uint16_t *)camera.fb->buf);

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
                    if (num_codes)
                        log_i("width %u height %u num_codes %d", fb->width, fb->height,num_codes);


                    for (int i = 0; i < num_codes; i++) {

                        quirc_extract(qr, i, code);
                        quirc_decode_error_t err = quirc_decode(code, data);
                        if (!err) {
                            log_i("payload '%s'", data->payload);
                            log_i("Version: %d", data->version);
                            log_i("ECC level: %c", "MLHQ"[data->ecc_level]);
                            log_i("Mask: %d", data->mask);
                            log_i("Length: %d", data->payload_len);
                            log_i("Payload: %s", data->payload);

                            // String payload = String((const char *)data.payload);
                            // CoreS3.Display.clear();
                            // String text = "QR Code: " + payload;
                            // CoreS3.Display.drawString(text.c_str(), 0, 0);
                            delay(3000);
                        } else {
                            log_e("quirc_decode_error_t %d", err);
                        }
                    }
#if 0
#endif
                }
                quirc_destroy(qr);
            }

            camera.free();
            // esp_camera_fb_return(fb);
        }
    } else {
        log_e("camera.get() fail");
        delay(1000);
    }
    yield();
}