#include "gpio_driver.h"
#include <Arduino.h>

void gpio_driver::pinMode(uint8_t pin, uint8_t mode) {
    ::pinMode(pin, mode);
}

void gpio_driver::digitalWrite(uint8_t pin, uint8_t value) {
    ::digitalWrite(pin, value); 
}
