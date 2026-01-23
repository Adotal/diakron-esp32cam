// main.cpp

#include <Arduino.h>
#include "esp_camera.h"
#include "WiFi.h"
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "HTTPClient.h"

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// ------------------------------CONSTANTS------------------
String lastPrediction;
const char *backendURL = "https://diakron-backend.onrender.com/analyze";

// Acces Point credentials
const char *SSID = "PWLAN_1";
const char *PASW = "244466666";

// Init wifi server port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM =
	R"rawliteral(
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
	<p id="result">Waiting...</p>

    <h2>ESP32-CAM Last Photo</h2>
    <p>It might take more than 5 seconds to capture a photo.</p>
    <p>
      <button onclick="capturePhoto()">CAPTURE PHOTO</button>
    </p>
  </div>
  <div><img src="" id="photo" width="70%"></div>
</body>
<script>

let gateway = `ws://${window.location.hostname}/ws`;
let websocket;
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}
  
function capturePhoto() {
	websocket.send("CAPT");
}

function onMessage(event) {

    // If it's binary → image
    if (event.data instanceof Blob) {
        const img = document.getElementById("photo");

        // Release previous image (important for memory)
        if (img.src) {
            URL.revokeObjectURL(img.src);
        }

    	console.log("New img");
        const url = URL.createObjectURL(event.data);
        img.src = url;
        return;
    } else if (event.data.startsWith('{"')) {
      document.getElementById("result").innerText =
        event.data;
	}

    // Otherwise it's text
    console.log("WS:", event.data);
}
</script>
</html>)rawliteral";

// --------------------------CAMERA CONFIG------------------
// OV2640 camera module

// OV2640 camera module
static camera_config_t camera_config = {

	.pin_pwdn = PWDN_GPIO_NUM,
	.pin_reset = RESET_GPIO_NUM,
	.pin_xclk = XCLK_GPIO_NUM,
	.pin_sccb_sda = SIOD_GPIO_NUM,
	.pin_sccb_scl = SIOC_GPIO_NUM,
	.pin_d7 = Y9_GPIO_NUM,
	.pin_d6 = Y8_GPIO_NUM,
	.pin_d5 = Y7_GPIO_NUM,
	.pin_d4 = Y6_GPIO_NUM,
	.pin_d3 = Y5_GPIO_NUM,
	.pin_d2 = Y4_GPIO_NUM,
	.pin_d1 = Y3_GPIO_NUM,
	.pin_d0 = Y2_GPIO_NUM,
	.pin_vsync = VSYNC_GPIO_NUM,
	.pin_href = HREF_GPIO_NUM,
	.pin_pclk = PCLK_GPIO_NUM,
	.xclk_freq_hz = 20000000,
	.ledc_timer = LEDC_TIMER_0,
	.ledc_channel = LEDC_CHANNEL_0,
	.pixel_format = PIXFORMAT_JPEG,
	// FRAMESIZE_UXGA (1600 x 1200)
	// FRAMESIZE_QVGA (320 x 240)
	// FRAMESIZE_CIF (352 x 288)
	// FRAMESIZE_VGA (640 x 480)
	// FRAMESIZE_SVGA (800 x 600)
	// FRAMESIZE_XGA (1024 x 768)
	// FRAMESIZE_SXGA (1280 x 1024)
	.frame_size = FRAMESIZE_VGA, // (640 x 480)
	.jpeg_quality = 10,
	.fb_count = 1,
	.grab_mode = CAMERA_GRAB_WHEN_EMPTY};

//-----------------GLOBA VARIABLES-------------------------
bool takeNewPhoto = false;

// ----------------------FUNCTIONS--------------------------

// Initialize WiFi
void initWiFi()
{
	WiFi.mode(WIFI_STA);

	WiFi.begin(SSID, PASW);
	Serial.print("Connecting to WiFi ..");
	while (WiFi.status() != WL_CONNECTED)
	{
		Serial.print('.');
		delay(1000);
	}
	Serial.println(WiFi.localIP());
}

void notifyClients(String state)
{
	ws.textAll(state);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
	AwsFrameInfo *info = (AwsFrameInfo *)arg;

	if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
	{
		String msg;
		for (size_t i = 0; i < len; i++)
			msg += (char)data[i];

		Serial.print("Received: ");
		Serial.println(msg);

		// --------- Detect message type ---------
		if (msg.startsWith("MOVE:"))
		{

			// Velocidad es 1 para baja y 2 para alta
			// dirc es UP o DW
			// Format: MOVE:dirc&velocidad

			// Obtiene sólo los datos
			String payload = msg.substring(5);

			// Obtiene índice del separador &
			int amp = payload.indexOf("&");

			String direction, velocidad;
			int velocidad_int;

			if (amp > 0)
			{
				// Obtiene dirección
				// UP or DW
				direction = payload.substring(0, amp);
				// 1 or 2
				velocidad = payload.substring(amp + 1);
				velocidad_int = velocidad.toInt();
			}

			Serial.print(direction);
			Serial.println(velocidad_int);

			notifyClients(direction);
			//   newRequest = true;
		}

		else if (msg.equals("CAPT"))
		{
			Serial.println("TAKING PHOTO...");

			// TAKE PHOTO
			takeNewPhoto = true;

			notifyClients("PHOTO TAKEN");
		}
		else if (msg.startsWith("ROT:"))
		{
		}
		else
		{
			Serial.println("Unknown message type.");
		}
	}
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
	switch (type)
	{
	case WS_EVT_CONNECT:
		Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
		// Notify client of motor current state when it first connects
		// notifyClients(direction);
		break;
	case WS_EVT_DISCONNECT:
		Serial.printf("WebSocket client #%u disconnected\n", client->id());
		break;
	case WS_EVT_DATA:
		handleWebSocketMessage(arg, data, len);
		break;
	case WS_EVT_PONG:
	case WS_EVT_ERROR:
		break;
	}
}

void initWebSocket()
{
	ws.onEvent(onEvent);
	server.addHandler(&ws);
}

// This function opens the image in FILE_PHOTOS and sends it to the backend
void sendPhotoToBackend(camera_fb_t *fb)
{

	// WiFiClientSecure client;
	// client.setInsecure(); // Nota: En producción usa certificados
	// Añadir en http.begin(client, backendURL)

	HTTPClient http;
	http.begin(backendURL);
	http.addHeader("Content-Type", "image/jpeg");
	http.setTimeout(4000);  // 4 seconds max to receive backend answer


	Serial.printf("Enviando %d bytes al backend...\n", fb->len);
	// Enviamos el buffer directamente de la RAM de la cámara
	int httpCode = http.sendRequest("POST", fb->buf, fb->len);

	if (httpCode > 0)
	{
		lastPrediction = http.getString();
		Serial.println("Backend Response: " + lastPrediction);
		// Send response to websocket
		notifyClients(lastPrediction);
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

void sendPhotoToWebSocket(camera_fb_t *fb)
{
	// Send JPEG buffer as binary WS frame
	ws.binaryAll(fb->buf, fb->len);
}

esp_err_t cameraCapture()
{
	// Dismiss last photo taken
	camera_fb_t *fb = esp_camera_fb_get();
	esp_camera_fb_return(fb);

	// capture a frame
	fb = esp_camera_fb_get();
	if (!fb)
	{
		Serial.print("Frame buffer could not be acquired");
		return ESP_FAIL;
	}

	// SEND TO WEBSOCKET SRVR
	sendPhotoToWebSocket(fb);

	// UPLOAD TO SERVER AND BACKEND
	sendPhotoToBackend(fb);

	// return the frame buffer back to be reused
	esp_camera_fb_return(fb);

	return ESP_OK;
}

void setup()
{
	Serial.begin(115200);

	if (psramFound)
		Serial.print("psramFound");

	initWiFi();
	initWebSocket();

	// Route for root / web page
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(200, "text/html", index_html); });

	// Start server
	server.begin();

	// Camera init
	esp_err_t err = esp_camera_init(&camera_config);
	if (err != ESP_OK)
	{
		Serial.printf("Camera init failed with error 0x%x", err);
		ESP.restart();
	}
}

void loop()
{

	if (takeNewPhoto)
	{
		takeNewPhoto = false;
		esp_err_t err = cameraCapture();
		if (err == ESP_OK)
		{
			Serial.print("Photo successfull");
		};
		delay(100);
	}

	ws.cleanupClients();
}