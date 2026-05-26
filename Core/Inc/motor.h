#ifndef MOTOR_H_
#define MOTOR_H_

#include <stdint.h>
#include "stm32f411xe.h"
#include <math.h>

void left_motor(int16_t speed);
void right_motor(int16_t speed);

#endif // MOTOR_H_