#include "interrupt.h"
#include "motor.h"     
#include "sensors.h"  

// Настраиваем "окно" скоростей. 
// Базовая должна быть ощутимо выше минимальной, чтобы было куда сбрасывать скорость.
#define SPEED_FAST   550 // Скорость для внешнего колеса при повороте
#define SPEED_BASE   450 // Базовая скорость для движения прямо
#define SPEED_MIN    350 // Минимальная скорость (ниже которой мотор глохнет)

static volatile uint32_t ticks = 0;
static volatile uint32_t press_time = 0;
static volatile uint8_t button_permission = 0;
static volatile uint8_t tim2_flag = 0;

// Память состояний
static uint8_t last_state = 0; // 0 - прямо, 1 - поворот влево, 2 - поворот вправо

static void led_toggle(void)
{
    GPIOC->ODR ^= GPIO_ODR_ODR_2;
}

void SysTick_Handler(void)
{
    static uint32_t led_counter = 0; 
    led_counter++;
    uint32_t current_interval = (button_permission == 1) ? 100U : 500U;

    if (led_counter >= current_interval)
    {
        led_toggle();     
        led_counter = 0;  
    }
    ticks++;
}

uint32_t get_ticks(void)
{
    return ticks;
}

void button_interrupt_init(void)
{
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
    MODIFY_REG(SYSCFG->EXTICR[13 / 4], 0xFU << ((13 % 4) * 4), 0x2U << ((13 % 4) * 4));
    SET_BIT(EXTI->IMR, EXTI_IMR_IM13);
    SET_BIT(EXTI->FTSR, EXTI_FTSR_TR13);
    SET_BIT(EXTI->RTSR, EXTI_RTSR_TR13);
    NVIC_SetPriority(EXTI15_10_IRQn, 2); 
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void EXTI15_10_IRQHandler(void)
{
    if (READ_BIT(EXTI->PR, EXTI_PR_PR13) != 0)
    {
        if (READ_BIT(GPIOC->IDR, GPIO_IDR_IDR_13) == 0)
        {
            press_time = ticks;
        }
        else
        {
            uint32_t release_time = ticks;
            if ((release_time - press_time) >= MIN_PRESS_DURATION)
            {
                button_permission = !button_permission; 
            }
        }
        SET_BIT(EXTI->PR, EXTI_PR_PR13);
    }
}

uint8_t get_button_permission(void)
{
    return button_permission;
}

void TIM2_IRQHandler(void)
{
    if (READ_BIT(TIM2->SR, TIM_SR_UIF)) 
    {
        CLEAR_BIT(TIM2->SR, TIM_SR_UIF); 
        tim2_flag = 1;

        if (button_permission == 0)
        {
            left_motor(0);
            right_motor(0);
            return;
        }

        uint8_t s_left   = read_right_sensor();  
        uint8_t s_middle = read_middle_sensor(); 
        uint8_t s_right  = read_left_sensor();   
        
        // 1. ИДЕАЛЬНО ПРЯМО
        if (!s_left && s_middle && !s_right)
        {
            last_state = 0;
            left_motor(SPEED_BASE);
            right_motor(SPEED_BASE);
        }
        
        // 2. ЛЕГКАЯ ДУГА ВЛЕВО - ускоряем правое, левое держим на грани сваливания
        else if (s_left && s_middle && !s_right)
        {
            last_state = 1;
            left_motor(SPEED_MIN);  
            right_motor(SPEED_FAST); 
        }
        
        // 3. ЛЕГКАЯ ДУГА ВПРАВО - ускоряем левое, правое держим на грани сваливания
        else if (!s_left && s_middle && s_right)
        {
            last_state = 2;
            left_motor(SPEED_FAST);
            right_motor(SPEED_MIN); 
        }
        
        // 4. КРУТАЯ ДУГА ВЛЕВО - тут останавливаем внутреннее колесо
        // Если инерция все равно выкидывает робота, попробуй тут left_motor(-SPEED_MIN)
        else if (s_left && !s_middle && !s_right)
        {
            last_state = 1;
            left_motor(0);    
            right_motor(SPEED_FAST);
        }
        
        // 5. КРУТАЯ ДУГА ВПРАВО
        // Аналогично, если заносит - используй реверс right_motor(-SPEED_MIN)
        else if (!s_left && !s_middle && s_right)
        {
            last_state = 2;
            left_motor(SPEED_FAST);
            right_motor(0);   
        }
        
        // 6. ПЕРЕКРЕСТОК
        else if ( (s_left && s_middle && s_right) || (s_left && !s_middle && s_right) )
        {
            last_state = 0; 
            left_motor(SPEED_BASE); 
            right_motor(SPEED_BASE);
        }
        
        // 7. ПОТЕРЯ ЛИНИИ (угол 90 градусов или вылет)
        // Вращаемся на месте на минимальной скорости, чтобы найти линию
        else if (!s_left && !s_middle && !s_right)
        {
            if (last_state == 1)
            {
                left_motor(-SPEED_MIN);
                right_motor(SPEED_MIN);
            }
            else if (last_state == 2)
            {
                left_motor(SPEED_MIN);
                right_motor(-SPEED_MIN);
            }
            else 
            {
                // Если слетели на прямой, сдаем чуть-чуть назад или просто стоим
                left_motor(0);
                right_motor(0);
            }
        }
    }
}

uint8_t get_tim2_flag(void)
{
    return tim2_flag;
}

void set_tim2_flag(uint8_t flag)
{
    tim2_flag = flag;
}