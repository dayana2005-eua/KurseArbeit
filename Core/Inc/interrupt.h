#ifndef INTERRUPT_H_
#define INTERRUPT_H_
#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f411xe.h"

#define MIN_PRESS_DURATION 50U

uint32_t get_ticks(void);
void button_interrupt_init(void);
uint8_t get_button_permission(void);
uint8_t get_tim2_flag(void);
void set_tim2_flag(uint8_t flag);


#endif