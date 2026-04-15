#pragma once
#include "../manager/sensor_manager.h"
#include "../communication/logger.h"

class SensorProtocol
{
private:
    SensorManager& manager;
public:
    SensorProtocol(SensorManager& sm);
    bool handle(char* command);
};

