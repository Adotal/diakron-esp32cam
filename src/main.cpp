// main.cpp
#include "main.h"

// -------------------------PIN DEFINITION & CONSTANTS--------------------------
#include "config/camera_pins.h"
const uint8_t hcsr04_echo_pins[4] = {P0, P1, P2, P3};
// Where to send the image
const char *backendURL = "https://diakron-backend.onrender.com/analyze";
// Acces Point credentials
const char *SSID = "INFINITUM6134";
const char *PASW = "DGkQb3J4DS";
// Private Key is a secret
extern const uint8_t private_key_start[] asm("_binary_secrets_private_key_ed25516_bin_start");
extern const uint8_t private_key_end[] asm("_binary_secrets_private_key_ed25516_bin_end");
// Define private key (its size is 32B)
const uint8_t *privateKey = private_key_start;

// --------------------------MOTOR DEFINITIONS--------------------------
Adafruit_MCP23X17 mcp;
mcp_driver interfaceI2C(mcp);
nema17 motorBase(interfaceI2C, 1, 2, 3);
stepper_28byj motorSensorINDU(interfaceI2C, 4, 5, 6, 7);
stepper_28byj motorSensorCAPC(interfaceI2C, 8, 9, 10, 11);
// TESTING MOVE ALL MOTORS AT THE SAME TIME
motor_manager testManager;
//-----------------GLOBAL VARIABLES-------------------------

/*	This array stores the information of trash thrown to show a QR in the
	HMI, so the user can earn points.
	Each cell stores 8 bits, 0-255 DEC.
	Structure (below are the indexes):
	[M][WM][P][WP][C][CW][G][GW][Timestamp][Nonce][Firm ED25516]
	 0  1   3  4   6  7   9  10  12     15  16 23  24        87
	M, P, C, G are the count of Metal, Plastic, Cardboard/Paper and Glass
	respectively, detected by the Segregator (Diakron),
	and MW, PW, CW, GW stands for WeightMetal, WeightPlastic,
	WeightCardPaper and WeightGlass

	Metal		- 1 BYTE
	WeightM		- 2 BYTES
	Plastic		- 1 BYTE
	WeightP		- 2 BYTES
	Card-Paper	- 1 BYTE
	WeightC		- 2 BYTES
	Glass		- 1 BYTE
	WeightG		- 2 BYTES
	Timestamp	- 4 BYTES
	Nonce		- 8 BYTES
	Firm		- 64 BYTES
	--------------------------
	TOTAL		- 88 BYTES
*/
uint8_t byteArrayQR[BYTES_QR];

/* This strcuture is made to overlay on byteArrayQR() and write on it
   in a fast-redable way (before I used directy pointers to the array
   but it's kind of unsafe and not so redable)
   The __attribute__((packed)) is to instruct the compiler to minimize
   the memmory occupied by the structure by removing
   padding bytes between data, on the right are array's indexes
*/

typedef struct __attribute__((packed))
{
	uint8_t countMetal;			// 0
	uint8_t weightMetal[2];		// 1
	uint8_t countPlastic;		// 3
	uint8_t weightPlastic[2];	// 4
	uint8_t countCardPaper;		// 6
	uint8_t weightCardPaper[2]; // 7
	uint8_t countGlass;			// 9
	uint8_t weightGlass[2];		// 10
	uint8_t timestamp[4];		// 12–15
	uint8_t nonce[8];			// 16–23
	uint8_t signature[64];		// 24–87 (ED25519)
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

// Flag to know if the materia type was identified
bool identified = false;

// Sensors input
bool inductive = false;
bool capacitive = false;

// Flag to take photo
bool takeNewPhoto = false;

// Wifi server port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Set your Static IP address
IPAddress local_IP(192, 168, 100, 128);
// Set your Gateway IP address
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

// Store AI segregation response
String lastPrediction;

// I2C expander on 0x20 address
PCF8574 pcf8574_0(0x20);

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

//------------------------FUNCTIONS PROTOTYPES

void sendfillLevels();

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

// Message received from HMI via websocket
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
			// Exampe of obtaining payload, vaues separated by &
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
		}

		else if (msg.equals("CAPT"))
		{
			Serial.println("TAKING PHOTO...");
			// TAKE PHOTO
			takeNewPhoto = true;
		}
		else if (msg.equals("FL"))
		{
			// Send Fill levels
			sendfillLevels();
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

		// Send fill levels on connect
		sendfillLevels();

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
	// Read sensors
	inductive = !digitalRead(GPIO_INDU);
	capacitive = !digitalRead(GPIO_CAPC);

	// GET MATERIA TYPE FROM AI MODEL
	JsonDocument doc;
	deserializeJson(doc, lastPrediction);
	String materiaType = doc["predicted"];

	if (inductive || materiaType.equals("metal"))
	{
		Serial.println("METAL");
		// notifyClients("METAL");
		// Increment one
		payloadQR->countMetal++;

		// SUMAR PESO
	}
	else if ((capacitive && materiaType.equals("glass")) || (capacitive && materiaType.equals("plastic")))
	{
		Serial.println("GLASS");
		// notifyClients("GLASS");
		// Increment one
		payloadQR->countGlass++;
	}
	else if (materiaType.equals("plastic") || (materiaType.equals("glass") && !capacitive))
	{
		Serial.println("PLASTIC");
		// notifyClients("PLASTIC");
		// Increment one
		payloadQR->countPlastic++;
	}
	else if (materiaType.equals("paper") || materiaType.equals("cardboard"))
	{
		Serial.println("PAPER/CARDBOARD");
		// notifyClients("PAPER/CARDBOARD");
		// Increment one
		payloadQR->countCardPaper++;
	}
	else
	{
		Serial.println("NO SÉ");
		// notifyClients("NO SE");
		// Not identified
		identified = false;
	}
}

void buildSendQRPayload()
{

	/*
		Build QR payload, sign it and send it to websocket
		QR structure is defined higher in code, but as a small reminder:
		[Metal][Plastic][Paper/Cardboard][Timestamp][Nonce][ED25519SIGN]
	*/

	// Get timestamp
	uint32_t tmp_millis = esp_log_timestamp();

Serial.println(tmp_millis);

	// Saving the number with LSB as MSB like twisting Endianess
	payloadQR->timestamp[0] = (uint8_t)(tmp_millis >> 24);
	payloadQR->timestamp[1] = (uint8_t)(tmp_millis >> 16);
	payloadQR->timestamp[2] = (uint8_t)(tmp_millis >> 8);
	payloadQR->timestamp[3] = (uint8_t)(tmp_millis);

	// Fill nonce of random numbers with ESP32 RNG
	esp_fill_random(payloadQR->nonce, sizeof(payloadQR->nonce));

	// Sign payload (BYTES_QR -64 because is ONLY the info to sign)
	Ed25519::sign(payloadQR->signature, privateKey, publicKey, byteArrayQR, BYTES_QR - 64);

	// Print if it was successfull
	Serial.print("QR BUILD SUCCESS: ");
	Serial.print(Ed25519::verify(payloadQR->signature, publicKey, byteArrayQR, BYTES_QR - 64));
	Serial.println("");

	// Print payloadQR content TESTING
	for (uint8_t i = 0; i < BYTES_QR; ++i)
	{
		Serial.print("[");
		Serial.print(byteArrayQR[i], HEX);
		Serial.print("] ");
	}

	// Send to websocket
	// ws.textAll("QR_BEGIN");
	ws.binaryAll(byteArrayQR, sizeof(byteArrayQR));
	// ws.textAll("QR_END");

	// Set the whole payload to 0
	memset(payloadQR, 0, sizeof(*payloadQR));
}

// Measure HC-SR04 via PCF8574 using pulseIn (high resolution but many I2C requests)
unsigned long measureDistancePulseIn(uint8_t echo_pin)
{
	// Generate trigger pulse using PCF8574
	pcf8574_0.digitalWrite(PCF_TRIG, LOW);
	delayMicroseconds(2);
	pcf8574_0.digitalWrite(PCF_TRIG, HIGH);
	delayMicroseconds(20); // 10 us is ok but 20 showed better reults
	pcf8574_0.digitalWrite(PCF_TRIG, LOW);

	// Measure ECHO pulse duration
	unsigned long duration = pcf8574_0.pulseIn(echo_pin, HIGH, 30000UL); // 30ms timeout
	unsigned long distanceCm = duration / 29 / 2;

	// Avoid overflow by validation of max depth
	if (distanceCm > binDepthCm)
		distanceCm = binDepthCm;

	return distanceCm;
}

// Send measured percentage levels of bin filling to HMI
void sendfillLevels()
{
	/*
		This array contains the payload to send Fill Levels as binary
		It is started by 'F','L' and followed by 4 bytes of each percentage
		of trash bin in the order: Metal, Plastic, Cardboard/Paper, Glass
	*/
	uint8_t fillLevels[6];
	fillLevels[0] = 'F';
	fillLevels[1] = 'L';

	Serial.print("FL");

	// Mease 3 times each one and get average, and sends it immediatly
	for (uint8_t i = 0; i < 4; ++i)
	{
		uint16_t sum = 0;
		for (uint8_t j = 0; j < 3; ++j)
		{
			// Get the sum of fill percentages
			// Commented for testing
			// sum += 100 - (measureDistancePulseIn(hcsr04_echo_pins[i]) * 100 / binDepthCm);
			sum = 1; // TESTING avoid over 0 division
		}

		// Get the average
		fillLevels[i + 2] = sum / 3;
		// fillLevels[i+2] = measureDistancePulseIn(hcsr04_echo_pins[i]);
		Serial.printf("%d, ", fillLevels[i + 2]);
	}

	// Sends fill levels
	ws.binaryAll(fillLevels, sizeof(fillLevels));
}

void setup()
{
	// TESTING
	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Disable brownout detector

	// TESTING
	Serial.begin(115200);

	// SENSORS PIN MODES
	pinMode(GPIO_INDU, INPUT_PULLUP);
	pinMode(GPIO_CAPC, INPUT_PULLUP);

	// Turn on INBOARD LED
	pinMode(GPIO_NORMAL_LED, OUTPUT);
	digitalWrite(GPIO_NORMAL_LED, 1);

	// HC-SR04 ECHO pins as input
	for (uint8_t i = 0; i < 4; ++i)
	{
		pcf8574_0.pinMode(hcsr04_echo_pins[i], INPUT);
	}
	// All 4 ultrasonic have same TRIGGER PIN
	pcf8574_0.pinMode(PCF_TRIG, OUTPUT);

	// Turn off sensors
	pcf8574_0.digitalWrite(PCF_TRIG, 0);

	// Initi I2C with custom pins
	Wire.begin(GPIO_I2C_SDA, GPIO_I2C_SCL);

	// Print message if psram
	if (psramFound)
		Serial.println("psramFound");

	// Set materialCount and weights to 0
	memset(payloadQR, 0, sizeof(*payloadQR));

	// Test values for weight TESTING
	payloadQR->weightMetal[0] = (uint8_t)(65535 >> 8);
	payloadQR->weightMetal[1] = (uint8_t)(65535);

	uint16_t pesoPlastico = 1300;
	payloadQR->weightPlastic[0] = (uint8_t)(pesoPlastico>> 8);
	payloadQR->weightPlastic[1] = (uint8_t)(pesoPlastico);

	// Start Wifi
	initWiFi();
	initWebSocket();

	// Start server
	server.begin();

	// Camera init
	esp_err_t err = esp_camera_init(&camera_config);
	if (err != ESP_OK)
	{
		Serial.printf("Camera init failed with error 0x%x", err);
		//ESP.restart();
	}

	// Initialize PCF8574
	if (!pcf8574_0.begin())
	{
		Serial.println(F("Could not initialize PCF8574!"));
	}

	// Initialize OLED UI
	if (!service_ui_init()){
		Serial.println(F("Could not initialize OLED UI!"));
	}

	// Initialize with default address 0x20 on the custom wire
	if(!mcp.begin_I2C(0x20, &Wire)){
		Serial.println(F("Could not initialize MCP23017!"));
	}
	// Initialize ALL motors
	motorBase.begin();
	motorSensorINDU.begin();
	motorSensorINDU.setDirection(true); // Set direction TESTING
	motorSensorCAPC.begin();
	motorSensorCAPC.setDirection(false);
	// TESTING MOVE ALL MOTORS AT THE SAME TIME
	testManager.addMotor(&motorBase);
	testManager.addMotor(&motorSensorINDU);
	testManager.addMotor(&motorSensorCAPC);

	delay(2000);
}

void loop()
{

	// If not identified is set to false in the last if-else block
	identified = true;
	if (takeNewPhoto)
	{
		takeNewPhoto = false;
		esp_err_t err = cameraCapture();
		if (err == ESP_OK)
		{
			Serial.println("Photo successfull");
		};

		// Select materia type and increment corresponding file
		selectFinalM();

		if (identified == false)
		{
			// SI no pudo, intenta otra vez
		}

		// If it was the last, send signed QR
		buildSendQRPayload();

		delay(100);
	}

	ws.cleanupClients();

	service_ui_update();

	testManager.update();

  // Cálculo de delay
  	unsigned long delayTime = (60000.0) / (STEPS_PER_REVOLUTION_SENSORS * RPM_SENSORS);
  
  	delay(delayTime);
}
