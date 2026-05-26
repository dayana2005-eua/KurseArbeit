#ifndef SENSORS_H_
#define SENSORS_H_
#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f411xe.h"

uint8_t read_left_sensor(void);
uint8_t read_middle_sensor(void);
uint8_t read_right_sensor(void);

#endif