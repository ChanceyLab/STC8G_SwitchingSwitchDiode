/*-----------------------------------------------------------------------
|                            FILE DESCRIPTION                           |
-----------------------------------------------------------------------*/
/*----------------------------------------------------------------------
  - File name     : init.c
  - Author        : zeweni
  - Update date   : 2020.01.11
  -	Copyright(C)  : 2020-2021 zeweni. All rights reserved.
-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------
|                               INCLUDES                                |
-----------------------------------------------------------------------*/
#include "main.h"
/*-----------------------------------------------------------------------
|                                 DATA                                  |
-----------------------------------------------------------------------*/

/*--------------------------------------------------------
| @Description: MCU peripheral initialization function   |
--------------------------------------------------------*/

static void STC8x_SYSCLK_Config(void);
static void STC8x_UART_Config(void);
static void STC8x_GPIO_Config(void);
static void STC8x_TIMER0_Config(void);
static void STC8x_TIMER2_Config(void);
static void STC8x_EXTI_Config(void);
static void STC8x_ADC_Config(void);
/*-----------------------------------------------------------------------
|                               FUNCTION                                |
-----------------------------------------------------------------------*/

/**
  * @name    STC8x_System_Init
  * @brief   MCU initialization function
  * @param   None
  * @return  None
***/
void STC8x_System_Init(void)
{
    DELAY_POS(); /* Power on stability delay */
    STC8x_SYSCLK_Config(); /* Initialize system clock */
    delay_init();
    STC8x_GPIO_Config();
    STC8x_UART_Config();
    STC8x_TIMER0_Config();
    //STC8x_TIMER2_Config();
    STC8x_ADC_Config();
    STC8x_EXTI_Config();
    /*
    	Add hardware driver initialization code here.
    */
    NVIC_GLOBAL_ENABLE();
}

/**
  * @name    STC8x_SYSCLK_Config
  * @brief   MCU SYSCLK initialization function
  * @param   None
  * @return  None
***/
static void STC8x_SYSCLK_Config(void)
{
    SYSCLK_InitType SYSCLK_InitStruct= {0}; /* Declare structure */
    SYSCLK_InitStruct.MCLKSrc = AUTO;
    SYSCLK_InitStruct.SCLKDiv = 0; /* if SCLKDiv = 0, Not output */
    SYSCLK_InitStruct.SCLKOutPin = SCLK_OUT_P16;
    SYSCLK_Init(&SYSCLK_InitStruct);
}

/**
  * @name    STC8x_GPIO_Config
  * @brief   MCU GPIO initialization function
  * @param   None
  * @return  None
***/
static void STC8x_GPIO_Config(void)
{
    GPIO_MODE_OUT_PP(GPIO_P1,Pin_0);		//输出		用	推挽输出
    GPIO_MODE_OUT_PP(GPIO_P1,Pin_1);		//输出		用	推挽输出
    GPIO_MODE_OUT_PP(GPIO_P1,Pin_6);		//输出		用	推挽输出
    GPIO_MODE_OUT_PP(GPIO_P1,Pin_7);		//输出		用	推挽输出

    GPIO_MODE_WEAK_PULL(GPIO_P5,Pin_5);		//按键		用	准双向IO口

    GPIO_MODE_WEAK_PULL(GPIO_P3,Pin_7);		//投切检测	用	准双向IO口
		
	RST_P54_ENABLE();
    /* Run lamp */
}

/**
  * @name    STC8x_TIMER0_Config
  * @brief   MCU TIMER0 initialization function
  * @param   None
  * @return  None
***/
static void STC8x_TIMER0_Config(void)
{
    TIMER_InitType TIMER_InitStruct= {0};
    TIMER_InitStruct.Mode = TIMER_16BitAutoReload;
    TIMER_InitStruct.Time = 1000;     //1ms
    TIMER_InitStruct.Run = ENABLE;
    TIMER0_Init(&TIMER_InitStruct);
    NVIC_TIMER0_Init(NVIC_PR2,ENABLE);
}

/**
  * @name    STC8x_TIMER2_Config
  * @brief   MCU TIMER2 initialization function
  * @param   None
  * @return  None
***/
static void STC8x_TIMER2_Config(void)
{
    TIMER_InitType TIMER_InitStruct= {0};

    TIMER_InitStruct.Mode = TIMER_16BitAutoReload;
    TIMER_InitStruct.Time = 100;     //0.1ms
    TIMER_InitStruct.Run = ENABLE;
    TIMER2_Init(&TIMER_InitStruct);
    NVIC_TIMER2_Init(ENABLE);
}

/**
  * @name    STC8x_UART_Config
  * @brief   MCU UART initialization function
  * @param   None
  * @return  None
***/
static void STC8x_UART_Config(void)
{
    UART_InitType UART_InitStruct = {0};

    /* UART1 TXD */
    GPIO_MODE_OUT_PP(GPIO_P3,Pin_1);
    /* UART1 RXD */
    GPIO_MODE_IN_FLOATING(GPIO_P3,Pin_0);

    UART_InitStruct.Mode = UART_8bit_BRTx;
    UART_InitStruct.BRTGen = UART_BRT_TIM1;
    UART_InitStruct.BRTMode = UART_BRT_1T;
    UART_InitStruct.BaudRate = 9600;
    UART_InitStruct.RxEnable = ENABLE;
    UART1_Init(&UART_InitStruct);
    NVIC_UART1_Init(NVIC_PR0,ENABLE);
}

/**
  * @name    STC8x_EXTI_Config
  * @brief   MCU EXTI initialization function
  * @param   None
  * @return  None
***/
static void STC8x_EXTI_Config(void)
{
    //在GPIO_MODE_IN_FLOATING模式下，IO口依然可以检测外部电平0or1
	GPIO_MODE_IN_FLOATING(GPIO_P3,Pin_2);
    NVIC_EXTI0_Init(EXTI_Tri_Edge,NVIC_PR1,ENABLE);
    GPIO_MODE_IN_FLOATING(GPIO_P3,Pin_3);
    NVIC_EXTI1_Init(EXTI_Tri_Edge,NVIC_PR1,ENABLE);
}

/**
  * @name    STC8x_ADC_Config
  * @brief   MCU ADC initialization function
  * @param   None
  * @return  None
***/
static void STC8x_ADC_Config(void)
{
    ADC_InitType ADC_InitStruct = {0};
    GPIO_MODE_IN_FLOATING(GPIO_P3,Pin_6);
    ADC_InitStruct.Power = ENABLE; 						// ADC power control bit
    ADC_InitStruct.Channel = ADC_Channel_P36_8G1K08_T; 	// ADC channel selection
    ADC_InitStruct.Speed = 0X0F; 						// The maximum ADC conversion speed (working clock frequency) is 0x0f
    ADC_InitStruct.Align = ADC_Right; 					// ADC data format alignment
    ADC_InitStruct.Run = ENABLE; 						//  ADC conversion operation control bit
    ADC_Init(&ADC_InitStruct);
    //NVIC_ADC_Init(NVIC_PR0,ENABLE);					//不启用中断
}

/*-----------------------------------------------------------------------
|                   END OF FLIE.  (C) COPYRIGHT zeweni                  |
-----------------------------------------------------------------------*/
