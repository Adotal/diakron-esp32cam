#include "command_router.h"

CommandRouter::CommandRouter(MotionProtocol& mp, StatusProtocol& sp, SensorProtocol& senp) : motion(mp), status(sp), sensor(senp)
{
}

bool CommandRouter::route(char* command)
{
    if(motion.handle(command)) return true;
    if(status.handle(command)) return true;
    if(sensor.handle(command)) return true;
    return false;
}