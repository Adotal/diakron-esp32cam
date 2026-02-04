// main.cpp

#include <Arduino.h>
#include "esp_camera.h"
#include "WiFi.h"
// For websocket
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
// To backend request
#include "HTTPClient.h"
// To parse backend response
#include "ArduinoJson.h"
// For binary payload
#include "stdint.h"
// For Ed25519 (signing qr payload)
#include "Crypto.h"
#include "Ed25519.h"

// PINS DEFINITION
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

#define BYTES_QR 80
#define GPIO_CAPC 12
#define GPIO_INDU 13

// ------------------------------CONSTANTS------------------
String lastPrediction;
const char *backendURL = "https://diakron-backend.onrender.com/analyze";

// Acces Point credentials
const char *SSID = "TOTALPLAY_E81F9F";
const char *PASW = "F3W411WTET";

// Init wifi server port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Private Key is a secret
extern const uint8_t private_key_start[] asm ("_binary_secrets_private_key_ed25516_bin_start");
extern const uint8_t private_key_end[] asm("_binary_secrets_private_key_ed25516_bin_end");
// Define private key (its size is 32B)
const uint8_t *privateKey = private_key_start;

//-----------------GLOBAL VARIABLES-------------------------

/*	This array stores the information of trash thrown to show a QR in the
	HMI, so the user can earn points.
	Each cell stores 8 bits, 0-255 DEC.
	Structure (below are the indexes):
	[M][P][C][G][Timestamp][Nonce][Firm ED25516]
	 0  1  2  3  4       7  8  15  16        79
	M, P, C, G are the count of Metal, Plastic, Cardboard/Paper and Glass
	respectively, detected by the Segregator (Diakron)

	M			- 1 BYTE
	P 			- 1 BYTE
	C			- 1 BYTE
	G 			- 1 BYTE
	Timestamp	- 4 BYTE
	Nonce		- 8 BYTE
	Firm		- 64 BYTES
	--------------------------
	TOTAL		- 80 BYTES
*/
uint8_t byteArrayQR[BYTES_QR];

/* This strcuture is made to overlay on byteArrayQR() and write on it
   in a fast-redable way (before I used directy pointers to the array
   but it's kind of unsafe and not so redable)
   The __attribute__((packed)) is to instruct the compiler to minimize
   the memmory occupied by the structure by removing
   padding bytes between data
*/

typedef struct __attribute__((packed))
{
	uint8_t countMetal;		// 0
	uint8_t countPlastic;	// 1
	uint8_t countCardPaper; // 2
	uint8_t countGlass;		// 3
	uint8_t timestamp[4];	// 4–7
	uint8_t nonce[8];		// 8–15
	uint8_t signature[64];	// 16–79 (ED25519)
} qr_payload_t;

// Overlay structure on array, enableing writing on it
qr_payload_t *payloadQR = (qr_payload_t *)byteArrayQR;

uint8_t publicKey[32] = {
	0x91,
	0x96,
	0x0d,
	0x0c,
	0x77,
	0x1c,
	0x93,
	0xe6,
	0x66,
	0xc0,
	0x73,
	0x43,
	0x6f,
	0x1b,
	0xb3,
	0xcf,
	0x0c,
	0xc2,
	0x32,
	0x4e,
	0xe9,
	0x82,
	0xd8,
	0xdf,
	0xf6,
	0xf2,
	0x86,
	0x49,
	0xb8,
	0x9b,
	0xea,
	0x3c,
};


bool inductivo = false;
bool capacitivo = false;
bool takeNewPhoto = false;

// Set your Static IP address
IPAddress local_IP(192, 168, 100, 128);
// Set your Gateway IP address
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

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


// ----------------------FUNCTIONS--------------------------

void createSendPayloadQR()
{
}

// Initialize WiFi
void initWiFi()
{

	// Configures static IP address
	if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
	{
		Serial.println("STA Failed to configure");
	}

	// WiFi.mode(WIFI_STA);
	WiFi.begin(SSID, PASW);
	Serial.print("Connecting to WiFi ..");
	while (WiFi.status() != WL_CONNECTED)
	{
		Serial.print('.');
		delay(1000);
	}
	Serial.println(WiFi.localIP());
}

// void notifyClients(String state)
// {
// 	ws.textAll(state);
// }

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
			// Obtiene sólo los datos
			// String payload = msg.substring(5);

			// Obtiene índice del separador &
			// int amp = payload.indexOf("&");

			// String direction, velocidad;
			// int velocidad_int;

			// if (amp > 0)
			// {
			// 	// Obtiene dirección
			// 	// UP or DW
			// 	direction = payload.substring(0, amp);
			// 	// 1 or 2
			// 	velocidad = payload.substring(amp + 1);
			// 	velocidad_int = velocidad.toInt();
			// }

			// Serial.print(direction);
			// Serial.println(velocidad_int);

			// notifyClients(direction);
			//   newRequest = true;
		}

		else if (msg.equals("CAPT"))
		{
			Serial.println("TAKING PHOTO...");

			// TAKE PHOTO
			takeNewPhoto = true;

			// notifyClients("PHOTO TAKEN");
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

	WiFiClientSecure client;
	client.setInsecure(); // Nota: En producción usa certificados
	// Añadir en http.begin(client, backendURL)

	HTTPClient http;
	http.begin(client, backendURL);
	http.addHeader("Content-Type", "image/jpeg");
	http.setTimeout(4000); // 4 seconds max to receive backend answer

	Serial.printf("Enviando %d bytes al backend...\n", fb->len);
	// Enviamos el buffer directamente de la RAM de la cámara
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

void sendPhotoToWebSocket(camera_fb_t *fb)
{
	// Notify that is sending an IMAGE
	// ws.textAll("IMG_BEGIN");

	// Send JPEG buffer as binary WS frame
	ws.binaryAll(fb->buf, fb->len);

	// NOtiify end of IMAGE
	// ws.textAll("IMG_END");
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
	// sendPhotoToWebSocket(fb);

	// UPLOAD TO SERVER AND BACKEND
	sendPhotoToBackend(fb);

	// return the frame buffer back to be reused
	esp_camera_fb_return(fb);

	return ESP_OK;
}

// Compare sensor with AI model and select a type
void selectFinalM()
{
	// void readSensors();

	inductivo = !digitalRead(GPIO_INDU);
	capacitivo = !digitalRead(GPIO_CAPC);
	// notifyClients("INDU: " + String(inductivo));
	// notifyClients("CAPC: " + String(capacitivo));

	// GET MATERIA TYPE
	JsonDocument doc;
	deserializeJson(doc, lastPrediction);
	String materiaType = doc["predicted"];

	if (inductivo || materiaType.equals("metal"))
	{

		Serial.println("METAL");
		// notifyClients("METAL");
	}
	else if ((capacitivo && materiaType.equals("glass")) || (capacitivo && materiaType.equals("plastic")))
	{
		Serial.println("GLASS");
		// notifyClients("GLASS");
	}
	else if (materiaType.equals("plastic") || (materiaType.equals("glass") && !capacitivo))
	{
		Serial.println("PLASTIC");
		// notifyClients("PLASTIC");
	}
	else if (materiaType.equals("paper") || materiaType.equals("cardboard"))
	{
		Serial.println("PAPER/CARDBOARD");
		// notifyClients("PAPER/CARDBOARD");
	}
	else
	{
		Serial.println("NO SÉ");
		// notifyClients("NO SE");
	}
}

void setup()
{
	Serial.begin(115200);

	// INICIO PRUEBAS ARRAYQR
	// Ed25519::derivePublicKey(publicKey, privateKey);

	Serial.println("Public KEY:");
	for (int i = 0; i < 32; ++i)
	{
		Serial.print("[");
		Serial.print(privateKey[i], HEX);
		Serial.print("] ");
	}

	// Empty byte array QR
	for (uint8_t i = 0; i < BYTES_QR; ++i)
		byteArrayQR[i] = 0;

	payloadQR->countMetal = 1;
	payloadQR->countPlastic = 2;
	payloadQR->countCardPaper = 3;
	payloadQR->countGlass = 1;

	uint32_t miliss = esp_log_timestamp();
	Serial.println(miliss);

	// esp_log_timestamp returns Little-Endian binary, converting to Bing-Endian
	for (int i = 3; i >= 0; --i)
	{
		// Deja el byte LSB y recorre
		payloadQR->timestamp[i] = (uint8_t)miliss;
		miliss = miliss >> 8; // Recorre ocho bits (lo mismo que dividir entre 256 DEC o 0xFF HEX
	}

	// Fill nonce of random numbers with ESP32 RNG
	esp_fill_random(payloadQR->nonce, sizeof(payloadQR->nonce));

	// Sign payload (16 because signature excludes signature)
	Ed25519::sign(payloadQR->signature, privateKey, publicKey, byteArrayQR, 16);

	Serial.print(Ed25519::verify(payloadQR->signature, publicKey, byteArrayQR, 16));

	// Imprime payloadQR
	for (uint8_t i = 0; i < BYTES_QR; ++i)
	{
		Serial.print("[");
		Serial.print(byteArrayQR[i]);
		Serial.print("] ");
	}

	// FIN PRUEBAS ARRAYQR

	// PIN MODES
	pinMode(GPIO_INDU, INPUT_PULLUP);
	pinMode(GPIO_CAPC, INPUT_PULLUP);

	if (psramFound)
		Serial.println("psramFound");

	initWiFi();
	initWebSocket();

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
			Serial.println("Photo successfull");
		};

		selectFinalM();

		delay(100);
	}

	ws.cleanupClients();
}