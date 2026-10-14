/*==============================================================================
 * Моргание светодиодом (пин B14) на К1921ВК035
 * Основано на официальной документации НИИЭТ
 *============================================================================*/

#include "K1921VK035.h"

// Определяем пин светодиода (теперь 14-й бит)
#define LED_PIN  (1 << 8)  // Пин A8
#define LED_PIN2 (1 << 9)  // Пин A9
#define LED_PIN3 (1 << 10)  // Пин A10
#define LED_PIN4 (1 << 11)  // Пин A11
#define LED_PIN5 (1 << 12)  // Пин A12
#define LED_PIN6 (1 << 13)  // Пин A13
#define LED_PIN7 (1 << 14)  // Пин A14
#define LED_PIN8 (1 << 15)  // Пин A15

const uint32_t led_pins[] = {LED_PIN,LED_PIN2,LED_PIN3,LED_PIN4,LED_PIN5,LED_PIN6,LED_PIN7,LED_PIN8};

// Функция задержки
void Delay(volatile uint32_t count)
{
    while (count--) {
        __NOP();  // Пустая операция, чтобы компилятор не оптимизировал цикл
    }
}

int main(void)
{
    // 1. Включаем тактирование GPIOB
    // RCU->HCLKCFG бит 1 = GPIOB (порт не меняется, меняется только пин внутри него)
    RCU->HCLKCFG |= (1 << 0);

    // 2. Снимаем сброс GPIOB
    // RCU->HRSTCFG бит 1 = GPIOB
    RCU->HRSTCFG |= (1 << 0);

    // 3. Разрешаем цифровую функцию пина B14
    // GPIOB->DENSET: запись 1 в бит n включает цифровую функцию вывода n
    GPIOA->DENSET = LED_PIN;
    GPIOA->DENSET = LED_PIN2;
    GPIOA->DENSET = LED_PIN3;
    GPIOA->DENSET = LED_PIN4;
     GPIOA->DENSET = LED_PIN5;
     GPIOA->DENSET = LED_PIN6;
     GPIOA->DENSET = LED_PIN7;
     GPIOA->DENSET = LED_PIN8;

    // 4. Разрешаем управление выходом пина B14
    // GPIOB->OUTENSET: запись 1 в бит n разрешает управление выходным уровнем
    GPIOA->OUTENSET = LED_PIN;
    GPIOA->OUTENSET = LED_PIN2;
    GPIOA->OUTENSET = LED_PIN3;
     GPIOA->OUTENSET = LED_PIN4;
     GPIOA->OUTENSET = LED_PIN5;
     GPIOA->OUTENSET = LED_PIN6;
     GPIOA->OUTENSET = LED_PIN7;
     GPIOA->OUTENSET = LED_PIN8;

    // 5. Устанавливаем режим выхода Push-Pull (по умолчанию 00, но явно укажем)
    // GPIOB->OUTMODE: 00 = Push-Pull для всех пинов.
    // Очищаем 2 бита, отвечающие за режим пина 14 (биты 14 и 14+16=30)
    GPIOA->OUTMODE &= ~(LED_PIN | (LED_PIN << 16));
    GPIOA->OUTMODE &= ~(LED_PIN2 | (LED_PIN2 << 16));
    GPIOA->OUTMODE &= ~(LED_PIN3 | (LED_PIN3 << 16));
      GPIOA->OUTMODE &= ~(LED_PIN4 | (LED_PIN4 << 16));
      GPIOA->OUTMODE &= ~(LED_PIN5 | (LED_PIN5 << 16));
        GPIOA->OUTMODE &= ~(LED_PIN6 | (LED_PIN6 << 16));
        GPIOA->OUTMODE &= ~(LED_PIN7 | (LED_PIN7 << 16));
          GPIOA->OUTMODE &= ~(LED_PIN8 | (LED_PIN8 << 16));

    GPIOA->DATAOUTTGL = LED_PIN;

    // Бесконечный цикл мигания
    while (1) {

        for (uint32_t i = 0; i < 8;i++)
        {
            GPIOA->DATAOUTTGL = led_pins[i];
            Delay(80000);
        }

        // 6. Переключаем состояние пина B14
        // GPIOB->DATAOUTTGL: запись 1 в бит n инвертирует вывод n
        //GPIOA->DATAOUTTGL = LED_PIN;
        //GPIOA->DATAOUTTGL = LED_PIN2;
        // 7. Задержка
        // При частоте 80 МГц значение 80000000 даст задержку примерно в 1 секунду
        //Delay(800000);
        //GPIOA->DATAOUTTGL = LED_PIN;
        //GPIOA->DATAOUTTGL = LED_PIN2;
        //Delay(800000);
    }

    return 0;
}
