/*==============================================================================
 * Вывод строки через UART1
 *------------------------------------------------------------------------------
 * НИИЭТ, Богдан Колбов <kolbov@niiet.ru>
 *==============================================================================
 * ДАННОЕ ПРОГРАММНОЕ ОБЕСПЕЧЕНИЕ ПРЕДОСТАВЛЯЕТСЯ «КАК ЕСТЬ», БЕЗ КАКИХ-ЛИБО
 * ГАРАНТИЙ, ЯВНО ВЫРАЖЕННЫХ ИЛИ ПОДРАЗУМЕВАЕМЫХ, ВКЛЮЧАЯ ГАРАНТИИ ТОВАРНОЙ
 * ПРИГОДНОСТИ, СООТВЕТСТВИЯ ПО ЕГО КОНКРЕТНОМУ НАЗНАЧЕНИЮ И ОТСУТСТВИЯ
 * НАРУШЕНИЙ, НО НЕ ОГРАНИЧИВАЯСЬ ИМИ. ДАННОЕ ПРОГРАММНОЕ ОБЕСПЕЧЕНИЕ
 * ПРЕДНАЗНАЧЕНО ДЛЯ ОЗНАКОМИТЕЛЬНЫХ ЦЕЛЕЙ И НАПРАВЛЕНО ТОЛЬКО НА
 * ПРЕДОСТАВЛЕНИЕ ДОПОЛНИТЕЛЬНОЙ ИНФОРМАЦИИ О ПРОДУКТЕ, С ЦЕЛЬЮ СОХРАНИТЬ ВРЕМЯ
 * ПОТРЕБИТЕЛЮ. НИ В КАКОМ СЛУЧАЕ АВТОРЫ ИЛИ ПРАВООБЛАДАТЕЛИ НЕ НЕСУТ
 * ОТВЕТСТВЕННОСТИ ПО КАКИМ-ЛИБО ИСКАМ, ЗА ПРЯМОЙ ИЛИ КОСВЕННЫЙ УЩЕРБ, ИЛИ
 * ПО ИНЫМ ТРЕБОВАНИЯМ, ВОЗНИКШИМ ИЗ-ЗА ИСПОЛЬЗОВАНИЯ ПРОГРАММНОГО ОБЕСПЕЧЕНИЯ
 * ИЛИ ИНЫХ ДЕЙСТВИЙ С ПРОГРАММНЫМ ОБЕСПЕЧЕНИЕМ.
 *
 *                              2018 АО "НИИЭТ"
 *==============================================================================
 */

//-- Includes ------------------------------------------------------------------
#include "plib035.h"
#include "retarget_conf.h"
#include "system_K1921VK035.h"

//-- Defines -------------------------------------------------------------------
volatile uint8_t print_flag = 0;
volatile uint8_t number_mess = 0;
volatile uint16_t ms_tick_count = 0;


void  ADC_Init_Channel0(void)
{
    RCU->HCLKCFG_bit.GPIOBEN = 1; //тактирование порта В
    RCU->HRSTCFG_bit.GPIOBEN = 1; //сброс порта В
    RCU->ADCCFG_bit.CLKEN = 1; // тактирование АЦП
    RCU->ADCCFG_bit.RSTDIS = 1; // сброс АЦП

    GPIOB->DENCLR = (1 << 0) | (1 << 1); //ОТКЛЮЧАЕМ цифровую функцию для порта В

    ADC->ACTL_bit.ADCEN = 1; //включаем ацп
    ADC->EMUX_bit.EM0 = 0; // программный запуск
    ADC->SEQ[0].SRQCTL_bit.RQMAX = 1; //2 измерение в очереди (вольт и ампер)
    ADC->SEQ[0].SRQSEL_bit.RQ0 = 0; // Канал 0 (PB0)
    ADC->SEQ[0].SRQSEL_bit.RQ1 = 0; // канал 1 (PB1)
    ADC->SEQEN_bit.SEQEN0 = 1; // Разрешаем секвенсор 0

    while (!ADC->ACTL_bit.ADCRDY) { } // ожидание готовности ацп


}

void ADC_Read_Volt_Amper(uint16_t *v,uint16_t *i)
{

    ADC->SEQSYNC_bit.SYNC0 = 1;
    ADC->SEQSYNC_bit.GSYNC = 1;


    while (ADC->SEQ[0].SFLOAD_bit.VAL < 2) {  }

    *v = (uint16_t)ADC->SEQ[0].SFIFO_bit.DATA;
    *i = (uint16_t)ADC->SEQ[0].SFIFO_bit.DATA;



}

//-- Peripheral init functions -------------------------------------------------
void uart_init(void)
{
    UART_Init_TypeDef UART_InitStruct;

    RCU_AHBClkCmd(RCU_AHBClk_GPIOB, ENABLE);
    RCU_AHBRstCmd(RCU_AHBRst_GPIOB, ENABLE);
    GPIO_AltFuncCmd(GPIOB, GPIO_Pin_8, ENABLE);
    GPIO_DigitalCmd(GPIOB, GPIO_Pin_8, ENABLE);

    // инициализация UART1
    RCU_UARTClkConfig(UART1_Num, RCU_PeriphClk_PLLClk, 0, DISABLE);
    RCU_UARTClkCmd(UART1_Num, ENABLE);
    RCU_UARTRstCmd(UART1_Num, ENABLE);
    UART_StructInit(&UART_InitStruct);
    UART_InitStruct.BaudRate = 115200;
    UART_InitStruct.Tx = ENABLE;
    UART_Init(UART1, &UART_InitStruct);
    UART_Cmd(UART1, ENABLE);
}

void periph_init(void)
{
    SystemCoreClockUpdate();
    // Заменяем настройку UART из функции retarget_init() на uart_init()
//    retarget_init();
    uart_init();
    printf("\nAll peripherals inited, SYSCLK = %3d MHz\n", (int)(SystemCoreClock / 1E6));
}

//-- Main ----------------------------------------------------------------------
int main(void)
{

    //  Разрешаем FPU — без этого любая float-инструкция = UsageFault
    SystemInit();
    ADC_Init_Channel0();
    periph_init();
    if (SysTick_Config(SystemCoreClock / 1000) != 0) {
            // Если сюда попали — ошибка (значение слишком велико для 24-битного таймера)
            printf("SysTick config failed!\n");
            while (1);
        }
    printf("Start timer!\n");

    while (1) {
        if (print_flag){
            print_flag = 0;
        uint16_t v_adc_code,i_adc_code;

        ADC_Read_Volt_Amper(&v_adc_code,&i_adc_code);

            float v_out =(3.3/4096)*(v_adc_code)*3;
            float i_out = (((3.3/4096)*(i_adc_code))/31)/0.005;
            printf("Voltege = %.3fv ,I = %.3fa!\n",v_out,i_out/100);
        }
    };
    return 0;
}

//-- IRQ handlers --------------------------------------------------------------

void SysTick_Handler(void)
{
    ms_tick_count++;
    if (ms_tick_count >= 1000){
    print_flag = 1;
    ms_tick_count = 0;
    }
}

