#ifndef SENSOR_H_
#define SENSOR_H_
#include <stdint.h>
#include "stm32f411xe.h"

uint8_t read_left_sensor(void);
uint8_t read_middle_sensor(void);
uint8_t read_right_sensor(void);

#endif