// --------------------------GENERAL SETTINGS--------------------------
#define SERIAL_BAUD_RATE 115200
#define version "1.4.2" // Version of the fimware DIAKRON Segregator. 

// -------------------------PIN DEFINITION & CONSTANTS--------------------------
#define CAMERA_MODEL_AI_THINKER
#define BYTES_QR 88
// Four HC-SR04 ultrasonic sensors, using same trigger pin, different echo
#define PCF_TRIG P4
// The can depth in centimeters (cm) to measure filling levels
#define binDepthCm 50

// ---------------------------MOTOR DEFINITIONS--------------------------
#define MAX_MOTORS_PER_MANAGER 26 // Limited to 26 axes (A-Z) for command parsing, plus one for safety margin
#define MAX_TRAVEL_STEPS_BASE 200   // limit max of steps
#define MAX_TRAVEL_STEPS_INDU 4096  // limit max of steps
#define MAX_TRAVEL_STEPS_CAPC 100   // limit max of steps

// ---------------------------OLED SCREEN--------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C         
#define OLED_RESET -1

// ---------------------------CAMERA CONFIG--------------------------
#define CAMERA_MODEL_AI_THINKER

// ---------------------------SENSORS CONFIG--------------------------
#define MAX_SENSORS_PER_MANAGER 2