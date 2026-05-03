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
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "drivers/buttonUI.h"
#include "core/interfaceUI.h"
// For camera capture
#include "config/camera_pins.h"
// For motor control
#include "communication/command_router.h"
#include "drivers/nema17.h"
#include "drivers/stepper_28byj.h"
#include "hal/gpio_driver.h"
#include "hal/mcp_driver.h"

// For Limits Switches
#include "core/axis.h"

// For defaults values
#include "config/defaults.h"

// For system
#include "system/system_manager.h"
#include "system/system_controller.h"

// Services
#include "services/camera_service.h"

// For sesors
#include "drivers/capacitive_sensor.h"
#include "drivers/inductive_sensor.h"
#include "core/sensor.h"
#include "manager/sensor_manager.h"
#include "protocols/sensor_protocol.h"

// For sync time
#include "time.h"