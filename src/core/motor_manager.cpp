#include "motor_manager.h"

void motor_manager::addMotor(motor* m) {
    if(count < MAX_MOTORS)
        motors[count++] = m;
}

void motor_manager::update() {
    for(uint8_t i = 0; i < count; i++)
        motors[i]->step();
}