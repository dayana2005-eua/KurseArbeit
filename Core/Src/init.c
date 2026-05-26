#include "init.h"
/**
 * @brief Инициализация портов для датчиков и двигателей
 * @note Функция использует пины PA0, PA1, PA4 для считывания датчиков линии
 *       PA5, PA6, PA7, PB5 для управления направлением вращения моторов
 *       PB3, PB10 для управления скважностью ШИМ      
 * @retval None
 */
void GPIO_init(void)
{
    /*
    Инициализация портов для датчиков и двигателей
    */
    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN); // Разрешено тактирование портов А
    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN); // Разрешено тактирование портов B

    // НАСТРОЙКА ДАТЧИКОВ (PA0, PA1, PA4) -> ВХОД

    CELAR_BIT(GPIOA->MODER, GPIO_MODER_MODE0); // Переводим в режим слушания порта PA0
    CLEAR_BIT(GPIOA->MODER, GPIO_MODER_MODE1); // Переводим в режим слуашния порта PA1
    CLEAR_BIT(GPIOA->MODER, GPIO_MODER_MODE4); // Переводим в режим слушания порта PA4

    CLEAR_BIT(GPIOA->PUPDR, GPIO_PUPDR_PUPD0); // Отключаем подтяжку порта PA0
    CLEAR_BIT(GPIOA->PUPDR, GPIO_PUPDR_PUPD1); // Отключаем подтяжку порта PA1
    CLEAR_BIT(GPIOA->PUPDR, GPIO_PUPDR_PUPD4); // Отключаем подтяжку порта PA4

    // НАСТРОЙКА ВЫХОДОВ КОНТРОЛЯ НАПРАВЛЕНИЯ ВРАЩЕНИЯ ((PA5, PA6) - ЛЕВЫЙ, (PA7, PB6) - ПРАВЫЙ) -> ВЫХОД

    CLEAR_BIT(GPIOA->MODER, GPIO_MODER_MODER5); // Очищаем соответствующие 2 бита порта PA5
    CLEAR_BIT(GPIOA->MODER, GPIO_MODER_MODER6); // Очищаем соответствующие 2 бита порта PA6
    CLEAR_BIT(GPIOA->MODER, GPIO_MODER_MODER7); // Очищаем соответствующие 2 бита порта PA7
    CLEAR_BIT(GPIOB->MODER, GPIO_MODER_MODER6); // Очищаем соответствующие 2 бита порта PB6

    SET_BIT(GPIOA->MODER, GPIO_MODER_MODE5_0); // Переводим порт PA5 в режим вывода
    SET_BIT(GPIOA->MODER, GPIO_MODER_MODE6_0); // Переводим порт PA6 в режим вывода
    SET_BIT(GPIOA->MODER, GPIO_MODER_MODE7_0); // Переводим порт PA7 в режим вывода
    SET_BIT(GPIOB->MODER, GPIO_MODER_MODE6_0); // Переводим порт PB6 в режим вывода

    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR5); // Отключаем выдачу сигнала портом PA5 воизбежание ложного срабатывания
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR6); // Отключаем выдачу сигнала портом PA6 воизбежание ложного срабатывания
    SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR7); // Отключаем выдачу сигнала портом PA7 воизбежание ложного срабатывания
    SET_BIT(GPIOB->BSRR, GPIO_BSRR_BR6); // Отключаем выдачу сигнала портом PB6 воизбежание ложного срабатывания

    // НАСТРОЙКА ШИМ ПОРТОВ (PB10 - ЛЕВЫЙ, PB3 - ПРАВЫЙ) -> ШИМ

    CLEAR_BIT(GPIOB->MODER, GPIO_MODER_MODE3 | GPIO_MODER_MODE10); // Очищаем предварительно биты для PB3 и PB10

    SET_BIT(GPIOB->MODER, GPIO_MODER_MODE3_1 | GPIO_MODER_MODE10_1); // Выставляем режим альтернативной функции для PB3 и PB10

    MODIFY_REG(GPIOB->AFR[0], 0xFU << (3 * 4), 1U << (3 * 4)); // PB3 управляется AFRL (индекс 3, смещение 12 бит)
    MODIFY_REG(GPIOB->AFR[1], 0xFU << ((10 - 8) * 4), 1U << ((10 - 8) * 4)); // PB10 управляется AFRH (индекс 10, смещение (10 - 8) * 4 = 8 бит)
}

/**
 * @brief Настройка тактирования контроллера на частоту 84 МГц
 * @retval None
 */
void RCC_init(void)
{
    SET_BIT(RCC->CR, RCC_CR_HSEON); // Включаем внешний кварцевый резонатор

    while(READ_BIT(RCC->CR, RCC_CR_HSERDY) == 0); // Ожидаем включения HSE

    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN); // Включаем тактирование контроллера питания
    MODIFY_REG(PWR->CR, PWR_CR_VOS, PWR_CR_VOS_1); 

    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_2WS); // Устанавливаем 2 цикла ожидания чтобы флеш память успевала за процессором

    // Формула: Частота = (HSE / M) * N / P
    // Для наших параметров: (8 МГц / 4) * 84 / 2 = 84 МГц

    MODIFY_REG(RCC->PLLCFGR, 
            RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLP | RCC_PLLCFGR_PLLQ | RCC_PLLCFGR_PLLSRC,
            (4U << RCC_PLLCFGR_PLLM_Pos)  | // M = 4
            (84U << RCC_PLLCFGR_PLLN_Pos) | // N = 84
            (0U << RCC_PLLCFGR_PLLP_Pos)  | // P = 2 (биты 00)
            (4U << RCC_PLLCFGR_PLLQ_Pos)  | // Q = 4
            RCC_PLLCFGR_PLLSRC_HSE);        // Источник — HSE
        
    SET_BIT(RCC->CR, RCC_CR_PLLON); // Включение блока PLL

    while(READ_BIT(RCC->CR, RCC_CR_PLLRDY) == 0); // Ожидаем включения блока PLL
    
    // Максимумы для шин: AHB = 84 МГц, APB1 = 42 МГц, APB2 = 84 МГц
    MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_CFGR_HPRE_DIV1);   // AHB = SYSCLK / 1 = 84 МГц
    MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1, RCC_CFGR_PPRE1_DIV2); // APB1 = AHB / 2 = 42 МГц
    MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, RCC_CFGR_PPRE2_DIV1); // APB2 = AHB / 1 = 84 МГц

    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL); // Переключение системного тактирования на PLL

    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL); // Ожидаем переход процессора на тактирование от PLL

}

/**
 * @brief Настройка TIMER2 для ШИМ-модуляции
 * @note Частота ШИМ 1 кГц. Частота TIMER2 1 МГц
 * @retval None
 */
void TIMER2_init(void)
{
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM2EN); // Включаем тактирование таймера 2 на шине APB1

    WRITE_REG(TIM2->PSC, 84 - 1); // Предделитель 84. Частота таймера 1 МГц
    WRITE_REG(TIM2->ARR, 1000 - 1); // Период ШИМ 1000 тиков. Частота ШИМ 1 кГц

    MODIFY_REG(TIM2->CCMR1, TIM_CCMR1_OC2M, TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2); // CH2 (Правый)
    MODIFY_REG(TIM2->CCMR2, TIM_CCMR2_OC3M, TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2); // CH3 (Левый)

    SET_BIT(TIM2->CCER, TIM_CCER_CC2E | TIM_CCER_CC3E); // Разрешаем выдачу ШИМ-сигнала на физические пины платы
    
    SET_BIT(TIM2->CR1, TIM_CR1_CEN); // Запускаем таймер TIM2
}

/**
 * @brief Настройка системного таймера
 */
void SysTick_init(void)
{
    WRITE_REG(SysTick->LOAD, (SystemCoreClock/1000U) - 1U); // 1 мс = 84000 тиков
    WRITE_REG(SysTick->VAL, 0U); // Обнуляем счетчик SysClock
    WRITE_REG(SysTick->CTRL, 0U); // Предварительная очистка регистра управления
    SET_BIT(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk); // Подключаем тактирование от ядра 
    SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk); // Разрешаем аппаратное прерывание при обнулении счетчика
    SET_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk); // Разрешаем работу
}