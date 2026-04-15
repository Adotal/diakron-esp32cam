#include "sensor_protocol.h"
#include <string.h>
#include <Arduino.h> // O <string> si es C++ estándar

SensorProtocol::SensorProtocol(SensorManager& sm) : manager(sm){}

bool SensorProtocol::handle(char* command)
{   
    if(strncmp(command, "SENSOR STATUS ", 14) == 0)
    {
        char sensorID = command[14];
        bool result = manager.readSensor(sensorID);
        String mensaje = "Sensor ";        
        mensaje += sensorID;
        mensaje += ": ";    
        mensaje += (result ? "ON" : "OFF");
        Logger::info(mensaje.c_str());
        return true;
    }
    if(strncmp(command, "SENSOR STATUS", 12) == 0)
    {
        manager.readAll();
        return true;
    }
    return false;
}