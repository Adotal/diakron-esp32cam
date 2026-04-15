#pragma once
#include "../protocols/motion_protocol.h"
#include "../protocols/status_protocol.h"
#include "../protocols/sensor_protocol.h"
/*
    This class stores all system protocols, associating them with their corresponding protocol depending on the received command.
    It uses the handle of each protocol to function.
    Whenever you want to use a new protocol in the system that requires a command to function, add the protocol as a parameter.
*/

class CommandRouter
{
private:
    MotionProtocol& motion;
    StatusProtocol& status;
    SensorProtocol& sensor;

public:
    CommandRouter(MotionProtocol& mp, StatusProtocol& sp, SensorProtocol& senp);

    bool route(char* command);
};