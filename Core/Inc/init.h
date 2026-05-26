#ifndef INIT_H_
#define INIT_H_

#include <stdint.h>
#include "stm32f411xe.h"

void GPIO_init(void);
void RCC_init(void);
void TIMER2_init(void);
void SysTick_init(void);

#endif /* INIT_H_ */
