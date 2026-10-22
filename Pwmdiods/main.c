#include "K1921VK035.h"
#include  "bsp.h"

#define PWM_MAX 999
#define STEP 20

volatile int32_t pwm_value = 0;
volatile int32_t pwm_dir = 1;
volatile int32_t diod_number = 0;


void reset_all_leds(void) {
    // Устанавливаем максимальное значение CMP (скважность 0%) для всех 6 каналов
    PWM0->CMPA_bit.CMPA = 0;
    PWM0->CMPB_bit.CMPB = 0;
    PWM1->CMPA_bit.CMPA = 0;
    PWM1->CMPB_bit.CMPB = 0;
    PWM2->CMPA_bit.CMPA = 0;
    PWM2->CMPB_bit.CMPB = 0;

    GPIOA->DATAOUTSET = (1 << 14) | (1 << 15);

}

void pwm_init(void)
{
    // 1. Тактирование
    RCU->HCLKCFG_bit.GPIOAEN = 1;  // GPIOA
    RCU->HRSTCFG_bit.GPIOAEN = 1;

    RCU->PCLKCFG_bit.PWM0EN = 1;
    RCU->PCLKCFG_bit.PWM1EN = 1;
    RCU->PCLKCFG_bit.PWM2EN = 1;
    RCU->PRSTCFG_bit.PWM0EN = 1;
    RCU->PRSTCFG_bit.PWM1EN = 1;
    RCU->PRSTCFG_bit.PWM2EN = 1;

    // Сброс предделителей ШИМ (биты 0, 1, 2)
    SIU->PWMSYNC_bit.PRESCRST = 0;

    // 2. Настройка GPIO для 6 каналов ШИМ (PA8 - PA13)
    uint32_t pwm_pins = (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) | (1 << 12) | (1 << 13);
    GPIOA->OUTENCLR   = pwm_pins;
    GPIOA->DENSET     = pwm_pins;
    GPIOA->ALTFUNCSET = pwm_pins;

    // 3. Настройка GPIO для 2 обычных диодов (PA14 - PA15)
    uint32_t gpio_pins = (1 << 14) | (1 << 15);
    GPIOA->DENSET   |= gpio_pins;
    GPIOA->OUTENSET |= gpio_pins;
    GPIOA->DATAOUTSET = gpio_pins; // Изначально погашены

    PWM_TypeDef *pwm_blocks[3] = {PWM0, PWM1, PWM2};
    // 4. Конфигурация всех трех блоков ШИМ (PWM0, PWM1, PWM2)
    // Обратите внимание: код одинаков для всех трех блоков, меняется только префикс
    for (uint8_t i = 0; i < 3; i++) {

        PWM_TypeDef *pwm = pwm_blocks[i];

        pwm->TBCTL_bit.CTRMODE   = PWM_TBCTL_CTRMODE_Up;
        pwm->TBCTL_bit.PRDLD     = 1; // Немедленная загрузка
        pwm->TBCTL_bit.HSPCLKDIV = PWM_TBCTL_HSPCLKDIV_Div10;
        pwm->TBCTL_bit.CLKDIV    = PWM_TBCTL_CLKDIV_Div1;


        pwm->CMPCTL_bit.SHDWAMODE = 1; // Немедленная загрузка CMPA
        pwm->CMPCTL_bit.SHDWBMODE = 1; // Немедленная загрузка CMPB

        pwm->TBPRD = PWM_MAX;
        pwm->CMPA_bit.CMPA = 0;
        pwm->CMPB_bit.CMPB = 0;

        // Настройка действий на выходах (Активный высокий уровень)
        pwm->AQCTLA_bit.ZRO = PWM_AQCTLA_ZRO_NoAction;
        pwm->AQCTLA_bit.PRD = PWM_AQCTLA_PRD_Clear;
        pwm->AQCTLA_bit.CAU = PWM_AQCTLA_CAU_Set;

        pwm->AQCTLB_bit.ZRO = PWM_AQCTLB_ZRO_NoAction;
        pwm->AQCTLB_bit.PRD = PWM_AQCTLB_PRD_Clear;
        pwm->AQCTLB_bit.CBU = PWM_AQCTLB_CBU_Set;

    }
    reset_all_leds();
}

// Настройка SysTick (10 мс)
void periph_init(void)
{
    SystemCoreClockUpdate();
    BSP_LED_Init(); // Инициализирует PA8-PA15 как GPIO (потом мы это переопределим)
    pwm_init();
    SysTick_Config(1000000);
}

int main(void)
{
    periph_init();

    // Запуск предделителей PWM0, PWM1, PWM2 одновременно
    SIU->PWMSYNC_bit.PRESCRST = (1 << 0) | (1 << 1) | (1 << 2);

    while (1) { }
    return 0;
}

// Обработчик прерывания
void SysTick_Handler(void)
{
 // 1. Изменяем pwm_value "пилой"
    pwm_value += STEP * pwm_dir;

    // 2. Проверяем достижение границ
    if (pwm_value >= PWM_MAX) {
        pwm_value = PWM_MAX;
        pwm_dir = -1;
    }
    else if (pwm_value <= 0) {
        pwm_value = 0;
        pwm_dir = 1;

        // Переключаемся на следующий диод
        diod_number++;
        if (diod_number >= 6) {
            diod_number = 0;
        }

        // ВАЖНО: перед тем как начать "дышать" следующим диодом,
        // гасим все предыдущие, чтобы горел только один!
        reset_all_leds();
    }

    // 3. Управляем текущим диодом
    switch (diod_number) {
        case 0: PWM0->CMPA_bit.CMPA = pwm_value; break;
        case 1: PWM0->CMPB_bit.CMPB = pwm_value; break;
        case 2: PWM1->CMPA_bit.CMPA = pwm_value; break;
        case 3: PWM1->CMPB_bit.CMPB = pwm_value; break;
        case 4: PWM2->CMPA_bit.CMPA = pwm_value; break;
        case 5: PWM2->CMPB_bit.CMPB = pwm_value; break;


    }
}
