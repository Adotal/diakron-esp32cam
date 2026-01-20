// main.cpp

// #include <Arduino.h>
// #include "esp_camera.h"
// #include <WiFi.h>

// TO TAKE PHOTO
#include "WiFi.h"
#include "driver/rtc_io.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "soc/rtc_cntl_reg.h" // Disable brownout problems
#include "soc/soc.h"          // Disable brownout problems
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <WiFiClientSecure.h>

// ---------------------CONTANTS----------------------

// Camera model
#define CAMERA_MODEL_AI_THINKER

// Pinout
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// ------------------------GLOBAL VARIABLES--------------------

String lastPrediction = "No data";
String lastConfidences = "";

// AP credentials
const char *ssid = "PWLAN_1";
const char *password = "244466666";

// PHOTOWEB
AsyncWebServer server(80);
bool takeNewPhoto = false;
#define FILE_PHOTO "/photo.jpg"

// Functions prototipes
void capturePhotoSaveSpiffs(void);
void sendPhotoToBackend();

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { text-align:center; }
    .vert { margin-bottom: 10%; }
    .hori{ margin-bottom: 0%; }
  </style>
</head>
<body>
  <div id="container">
  <h3>AI Result</h3>
<pre id="result">Waiting...</pre>

    <h2>ESP32-CAM Last Photo</h2>
    <p>It might take more than 5 seconds to capture a photo.</p>
    <p>
      <button onclick="capturePhoto()">CAPTURE PHOTO</button>
    </p>
  </div>
  <div><img src="saved-photo" id="photo" width="70%"></div>
</body>
<script>
  
  function capturePhoto() {
  fetch("/capture")
    .then(() => {
      setTimeout(() => {
        document.getElementById("photo").src =
          "saved-photo?t=" + Date.now();
      }, 200); // espera a que se guarde la nueva foto
    });
  }

  function loadResult() {
  fetch("/result")
    .then(r => r.json())
    .then(j => {
      document.getElementById("result").innerText =
        JSON.stringify(j, null, 2);
    });
  }
  setInterval(loadResult, 3000);
</script>
</html>)rawliteral";

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  } else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }

  // Print ESP32 Local IP Address
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());

  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // OV2640 camera module
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  // config.frame_size = FRAMESIZE_QVGA; // 320x240
  config.frame_size = FRAMESIZE_VGA; // (640 x 480)
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }

  // Sensor config
  sensor_t *s = esp_camera_sensor_get();

  // Auto controls (mantener ON)
  s->set_gain_ctrl(s, 1);     // AGC
  s->set_exposure_ctrl(s, 1); // AEC
  s->set_awb_gain(s, 1);      // AWB

  // Imagen
  s->set_brightness(s, 0); // -2 .. 2
  s->set_contrast(s, 1);   // -2 .. 2
  s->set_saturation(s, 0); // -2 .. 2
  s->set_sharpness(s, 1);  // 0 .. 3

  // Calidad
  s->set_denoise(s, 1); // reduce puntos verdes
  s->set_quality(s, 12);

  // Correcciones ópticas
  s->set_lenc(s, 1);     // corrección de lente
  s->set_colorbar(s, 0); // OFF

   // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });

  server.on("/capture", HTTP_GET, [](AsyncWebServerRequest *request) {
    takeNewPhoto = true;
    request->send(200, "text/plain", "Taking Photo");
  });

  server.on("/saved-photo", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, FILE_PHOTO, "image/jpg", false);
  });

  server.on("/result", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", lastPrediction);
  });

  // Start server
  server.begin();
}

void loop() {
  if (takeNewPhoto) {
    capturePhotoSaveSpiffs();
    sendPhotoToBackend();
    takeNewPhoto = false;
  }
  delay(1);
}

// Capture Photo and Save it to SPIFFS
void capturePhotoSaveSpiffs(void) {

   // The first photo is dismiss because of stabilization
  // for (int i = 0; i < 2; i++) {
  //   camera_fb_t *fb = esp_camera_fb_get();
  //   if (fb)
  //     esp_camera_fb_return(fb);
  //   delay(200);
  // }

  // Dismiss previous frame
  camera_fb_t *fb = esp_camera_fb_get();
  if (fb)
    esp_camera_fb_return(fb);
  delay(50);

  Serial.println("Taking photo...");
  // Taking real photo
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Open SPIFFSS file
  File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file");
    esp_camera_fb_return(fb);
    return;
  }
  // Write the image
  file.write(fb->buf, fb->len);

  // Close file
  file.close();
  esp_camera_fb_return(fb);

  Serial.println("Photo captured and saved in");
  Serial.print(FILE_PHOTO);
  Serial.print(" - Size: ");
  Serial.print(file.size());
  Serial.println(" bytes");
}

// This function opens the image in FILE_PHOTOS and sends it to the backend
void sendPhotoToBackend() {

  // Open photo in (r)ead mode
  File file = SPIFFS.open(FILE_PHOTO, "r");
  if (!file) {
    Serial.println("Failed to open photo");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, "https://diakron-backend.onrender.com/analyze");
  http.addHeader("Content-Type", "image/jpeg");

  int httpCode = http.sendRequest("POST", &file, file.size());

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("Backend response:");
    Serial.println(payload);
    lastPrediction = payload;
  } else {
    Serial.printf("HTTP error: %d\n", httpCode);
  }

  // Close file and end connection
  file.close();
  http.end();
}