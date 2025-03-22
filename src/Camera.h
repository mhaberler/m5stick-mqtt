#ifndef CAMERA_H
#define CAMERA_H

#include "M5Unified.h"
#include "esp_camera.h"

class Camera {
private:
public:
    camera_fb_t* fb;
    sensor_t* sensor;
    camera_config_t* config;
    bool begin();
    bool get();
    bool free();
};

#endif