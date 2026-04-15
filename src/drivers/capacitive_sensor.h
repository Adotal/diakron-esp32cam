#pragma once
#include "../hal/IPinDriver.h"
#include "../core/sensor.h"

class CapacitiveSensor : public Sensor
{
private:
    uint8_t pin;
public:
    CapacitiveSensor(IPinDriver& drv, uint8_t sensorPin);
    void begin() override;
    bool read() override;
};