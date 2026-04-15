#include "inductive_sensor.h"
#include <Arduino.h>

InductiveSensor::InductiveSensor(IPinDriver& drv, uint8_t sensorPin) : Sensor(&drv), pin(sensorPin) {
    this->driver = &drv;
}

void InductiveSensor::begin()
{
    // Set pin mode to INPUT
    driver->pinMode(pin, INPUT_PULLUP);
}

bool InductiveSensor::read()
{
    // Read the pin value and return true if it's LOW (activated)
    return driver->digitalRead(pin) == LOW;
}