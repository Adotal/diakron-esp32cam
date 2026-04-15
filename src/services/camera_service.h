#pragma once
#include <Arduino.h>
#include <esp_camera.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#define CAMERA_MODEL_AI_THINKER
#include "../config/camera_pins.h"
class CameraService
{
private:
    camera_config_t config;
    String lastPrediction;
    bool newPhotoAvailable;

    const char* backendURL;

    void sendPhotoToBackend(camera_fb_t* fb);

public:
    CameraService(const char* url);

    void init();

    void requestCapture();          // activate photo capture
    bool hasNewResult();            // to know if there is already a result
    String getPrediction();         // Get AI result
};