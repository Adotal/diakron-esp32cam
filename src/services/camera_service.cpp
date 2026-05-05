#include "camera_service.h"
CameraService::CameraService(const char *url)
{
    backendURL = url;
    newPhotoAvailable = false;
}

void CameraService::init()
{
    camera_config_t config;

    // ---------------- PIN CONFIG ----------------
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;

    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d0 = Y2_GPIO_NUM;

    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;

    // ---------------- CLOCK ----------------
    config.xclk_freq_hz = 20000000;
    config.ledc_timer = LEDC_TIMER_0;
    config.ledc_channel = LEDC_CHANNEL_0;

    // ---------------- FORMAT ----------------
    config.pixel_format = PIXFORMAT_JPEG;

    // ---------------- QUALITY ----------------
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    // ---------------- INIT CAMERA ----------------
    esp_err_t err = esp_camera_init(&config);

    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed: 0x%x\n", err);
        return;
    }

    Serial.println("Camera initialized");
}

void CameraService::requestCapture()
{
    newPhotoAvailable = false;

    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb)
    {
        Serial.println("Camera capture failed");
        return;
    }

    sendPhotoToBackend(fb);

    esp_camera_fb_return(fb);

    newPhotoAvailable = true;
}

void CameraService::sendPhotoToBackend(camera_fb_t* fb)
{
	WiFiClientSecure client;
	client.setInsecure(); // TESTING Use ceritifcates in production

	HTTPClient http;
	http.begin(client, backendURL);
	http.addHeader("Content-Type", "image/jpeg");
	http.setTimeout(30000); // 4 seconds max to receive backend answer

	Serial.printf("Enviando %d bytes al backend...\n", fb->len);
	// We send the buffer directly from the camera's RAM
	int httpCode = http.sendRequest("POST", fb->buf, fb->len);

	if (httpCode > 0)
	{
		lastPrediction = http.getString();
		Serial.println("Backend Response: " + lastPrediction);
		// Send response to websocket
		// notifyClients(lastPrediction);

		// const char *lastJSON = lastPrediction.c_str();
		// // Identify materia type
		// cJSON *jsonRoot = cJSON_Parse(lastJSON);
		// if (jsonRoot == NULL)
		// {
		// 	printf("Error before: %s\n", cJSON_GetErrorPtr());
		// 	return;
		// }
		// cJSON *jsonmateriaType = cJSON_GetArrayItem(jsonRoot, 2);
	}
	else
	{
		Serial.print("Error: " + String(httpCode) + " - ");
		// Especify error
		switch (httpCode)
		{
		case HTTPC_ERROR_CONNECTION_REFUSED:
			Serial.println("CONN_REFUSED");
			break;
		case HTTPC_ERROR_CONNECTION_LOST:
			Serial.println("CONN_LOST");
			break;
		case HTTPC_ERROR_NO_STREAM:
			Serial.println("NO_STREAM");
			break;
		case HTTPC_ERROR_READ_TIMEOUT:
			Serial.println("TIMEOUT");
			break;
		default:
			Serial.println("BACKEND_ERR");
			break;
		}
	}
	http.end();
}

bool CameraService::hasNewResult()
{
    return newPhotoAvailable;
}

String CameraService::getPrediction()
{
    newPhotoAvailable = false;
    return lastPrediction;
}

