#pragma once

enum class SystemState
{
    BOOT,
    IDLE,
    HOMING,
    RUNNING,
    ERROR,
    ESTOP,
    DETECTED,
    CAPTURE,
    WAIT_BACKEND,
    MOVE_SENSORS,
    READ_SENSORS,
    DECIDE,
    MOVE_BIN,
    DUMP,
    RETURN_HOME,
    GENERATE_QR,
    SHOW_QR
};

enum class SystemError
{
    NONE,
    NOT_HOMED,
    LIMIT_HIT,
    MOTOR_FAULT,
    UNKNOWN_COMMAND
};

