#include <M5CoreS3.h>
#include <Camera.h>
#include <quirc.h>
// #include "esp_camera.h"

Camera camera;

struct quirc_code *code;
struct quirc_data *data;

// Convert RGB565 to 8-bit grayscale using luminance weights
static inline void rgb565_to_grayscale(const uint16_t* input, uint8_t* output, int width, int height) {
    for (int i = 0; i < width * height; i++) {
        uint16_t rgb = input[i];

        // Extract RGB components
        uint8_t r = ((rgb >> 11) & 0x1F) << 3;  // 5 bits to 8 bits
        uint8_t g = ((rgb >> 5) & 0x3F) << 2;   // 6 bits to 8 bits
        uint8_t b = (rgb & 0x1F) << 3;          // 5 bits to 8 bits

        // Calculate grayscale using luminance weights
        output[i] = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);
    }
}

// Convert RGB565 to 8-bit grayscale
static inline void  convertToGrayscale(uint16_t* src_buffer, uint8_t* dst_buffer, int width, int height) {
    for (int i = 0; i < width * height; i++) {
        uint16_t pixel = src_buffer[i];
        uint8_t R = (pixel & 0xF800) >> 11;
        uint8_t G = (pixel & 0x07E0) >> 5;
        uint8_t B = pixel & 0x001F;
        float gray = 0.2126 * (R / 31.0f) + 0.7152 * (G / 63.0f) + 0.0722 * (B / 31.0f);
        dst_buffer[i] = round(gray * 255);
    }
}

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
    // Assume camera is set to YUV422
    if (camera.get()) {
        camera_fb_t *fb = camera.fb;
        if (fb) {
            // CoreS3.Display.pushGrayscaleImage(0, 0, CoreS3.Display.width(), CoreS3.Display.height(),
            //                          (uint8_t *)camera.fb->buf, lgfx::v1::grayscale_8bit, TFT_WHITE, TFT_BLACK);
            CoreS3.Display.pushImage(0, 0, CoreS3.Display.width(), CoreS3.Display.height(),
                                     (uint16_t *)camera.fb->buf);

            int width = fb->width;
            int height = fb->height;
            struct quirc *qr = quirc_new();

            if (qr && quirc_resize(qr, width, height) >= 0) {
                uint8_t *image = quirc_begin(qr, &width, &height);
                if (image) {

                    rgb565_to_grayscale((const uint16_t*)fb->buf, image,  width, height);
                    // convertToGrayscale((uint16_t*)fb->buf, image,  width, height);

                    // uint16_t *pixels = (uint16_t *) fb->buf;
                    // for(int i = 0; i < width * height; i++) {
                    //     // image[i] = fb->buf[2 * i]; // Y channel
                    //     image[i]
                    // }
                    // memcpy(image, fb->buf, fb->len);

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
                            CoreS3.Display.clear();
                            String text = "QR Code: " + payload;
                            CoreS3.Display.setTextDatum(middle_center);
                            CoreS3.Display.drawString(text.c_str(), 0, 0);
                            delay(3000);
                        } else {
                            log_e("quirc_decode_error_t %d", err);
                        }
                    }
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