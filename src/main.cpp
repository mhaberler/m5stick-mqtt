#include <M5CoreS3.h>
#include <Camera.h>
#include <quirc.h>

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

    // CoreS3.Speaker.begin();

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
    if (camera.get()) {
        camera_fb_t *fb = camera.fb;
        if (fb) {
            if (camera.config->pixel_format == PIXFORMAT_RGB565) {
                CoreS3.Display.pushImage(0, 0, CoreS3.Display.width(), CoreS3.Display.height(),
                                         (uint16_t *)camera.fb->buf);
            }
            if (camera.config->pixel_format == PIXFORMAT_GRAYSCALE) {
                CoreS3.Display.pushGrayscaleImage(0, 0, CoreS3.Display.width(), CoreS3.Display.height(),
                                                  (uint8_t *)camera.fb->buf, lgfx::v1::grayscale_8bit, TFT_WHITE, TFT_BLACK);
            }

            int width = fb->width;
            int height = fb->height;
            struct quirc *qr = quirc_new();

            if (qr && quirc_resize(qr, width, height) >= 0) {
                uint8_t *image = quirc_begin(qr, &width, &height);
                if (image) {
                    if (camera.config->pixel_format == PIXFORMAT_RGB565) {
                        rgb565_to_grayscale((const uint16_t*)fb->buf, image,  width, height);
                    }
                    if (camera.config->pixel_format == PIXFORMAT_GRAYSCALE) {
                        memcpy(image, fb->buf, fb->len);
                    }
                    quirc_end(qr);
                    int num_codes = quirc_count(qr);
                    if (num_codes) {
                        // CoreS3.Speaker.tone(1000, 100);
                        log_i("width %u height %u num_codes %d", fb->width, fb->height,num_codes);
                    }

                    for (int i = 0; i < num_codes; i++) {
                        quirc_extract(qr, i, code);
                        quirc_decode_error_t err = quirc_decode(code, data);
                        if (err == QUIRC_ERROR_DATA_ECC) {
                            quirc_flip(code);
                            err = quirc_decode(code, data);
                        }
                        if (!err) {
                            log_i("payload '%s'", data->payload);
                            log_i("Version: %d", data->version);
                            log_i("ECC level: %c", "MLHQ"[data->ecc_level]);
                            log_i("Mask: %d", data->mask);
                            log_i("Length: %d", data->payload_len);
                            log_i("Payload: %s", data->payload);

                            String payload = String((const char *)data->payload);
                            String text = "QR Code: " + payload;

                            CoreS3.Display.clear();
                            CoreS3.Display.setTextColor(GREEN);
                            // CoreS3.Display.setTextDatum(middle_center);
                            CoreS3.Display.setFont(&fonts::FreeSans12pt7b);
                            CoreS3.Display.setTextSize(1);

                            CoreS3.Display.setCursor(0, 20);
                            CoreS3.Display.printf("QR: %s\n\n", payload.c_str());
                            CoreS3.Display.printf("Version: %d\n", data->version);
                            CoreS3.Display.printf("ECC level: %c\n", "MLHQ"[data->ecc_level]);
                            CoreS3.Display.printf("Mask: %d\n", data->mask);
                            CoreS3.Display.printf("Length: %d\n", data->payload_len);

                            delay(3000);
                        } else {
                            CoreS3.Display.clear();
                            CoreS3.Display.setTextColor(RED);
                            // CoreS3.Display.setTextDatum(middle_center);
                            CoreS3.Display.setFont(&fonts::Orbitron_Light_24);
                            CoreS3.Display.setTextSize(1);
                            // CoreS3.Display.setTextDatum(middle_center);

                            CoreS3.Display.setCursor(20, 20);
                            CoreS3.Display.printf("decode error: %d\n",err);
                            delay(500);

                        }
                    }
                }
                quirc_destroy(qr);
            }
            camera.free();
        }
    } else {
        log_e("camera.get() fail");
        delay(1000);
    }
    yield();
}