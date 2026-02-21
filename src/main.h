// main.cpp

#include <Arduino.h>
#include "config/pins.h"
#include "esp_camera.h"
#include "WiFi.h"
// For websocket
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
// For backend request
#include "HTTPClient.h"
// For parsing backend response
#include "ArduinoJson.h"
// For binary payload
#include "stdint.h"
// For Ed25519 (signing qr payload)
#include "Crypto.h"
#include "Ed25519.h"
// For disabling brownout detecor TESTING
#include "soc/soc.h"		  // Access system control
#include "soc/rtc_cntl_reg.h" // Access RTC control registers)
// For I2C (PCF8574 OR MCP23017) as GPIO expansor
#include "Wire.h"
#include "PCF8574.h"
// For UI OLED
#include "ui/service_ui.h"
// For camera capture
#include "config/camera_pins.h"
// For motor control
#include "core/motor_manager.h"
#include "drivers/nema17.h"
#include "drivers/stepper_28byj.h"
#include "hal/gpio_driver.h"
#include "hal/mcp_driver.h"

// For defaults values
#include "config/defaults.h"