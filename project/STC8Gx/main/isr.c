/*-----------------------------------------------------------------------
|                            FILE DESCRIPTION                           |
-----------------------------------------------------------------------*/
/*----------------------------------------------------------------------
  - File name     : isr.c
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
/* None */
/*-----------------------------------------------------------------------
|                               FUNCTION                                |
-----------------------------------------------------------------------*/


u8 G_KEY_VALUE = 1;						//键值1-10
u8 Timer0_ISRQ_Length_30 = 30;

u8 FLAG_EXTISource_0 = 0;				//外部0中断进入标志位
u8 FLAG_EXTISource_1 = 0;				//外部1中断进入标志位

u8 AB_Delat_Time_ms = 0;				//两相时间差
u8 AB_Delat_Time_EN = 0;				//相序测量使能
/**
  * @name    TIMER0_ISRQ_Handler--IO OUTPUT
  * @brief   MCU TIMER0 Interrupt request service function
  * @param   None
  * @return  None
***/
void TIMER0_ISRQ_Handler(void)
{
    static u8 key_up = 0;				//按键松开标志位
    static timer0_count_2 = 0;
//	if(++timer0_count_1 >= (G_KEY_VALUE * Timer0_ISRQ_Length_step))
//	{
//		timer0_count_1 = 0;
//		P16 = ~P16;
//		P17 = ~P17;
//	}

    //按键检测部分
    if(++timer0_count_2 >= Timer0_ISRQ_Length_30)
    {
        timer0_count_2 = 0;
        if(G_KEY_VALUE > 10) 	G_KEY_VALUE = 1;
        if(key_up && P55 == 0)
        {
            key_up=0;
            G_KEY_VALUE ++;
        }
        else if(P55 == 1)		key_up=1;
    }

    //外部中断计时部分
    if(AB_Delat_Time_EN && (FLAG_EXTISource_0 ^ FLAG_EXTISource_1))
    {
        AB_Delat_Time_ms ++;
    }
}

/**
  * @name    TIMER2_ISRQ_Handler--TO BE A TIMER
  * @brief   MCU TIMER2 Interrupt request service function
  * @param   None
  * @return  None
***/
void TIMER2_ISRQ_Handler(void)
{


    TIMER2_CLEAR_FLAG();				//TIMER2需要手动清除标志位
}

/**
  * @name    UART1_ISRQ_Handler
  * @brief   MCU UART1 Interrupt request service function
  * @param   None
  * @return  None
***/
void UART1_ISRQ_Handler(void)
{
    if(UART1_GET_RX_FLAG())
    {
        UART1_CLEAR_RX_FLAG();
        Auto_RST_download();
    }
    else if(UART1_GET_TX_FLAG())
    {
        UART1_CLEAR_TX_FLAG();
        UART1_CLEAR_BUSY_FLAG();
    }
}

/**
  * @name    EXTI0_ISRQ_Handler
  * @brief   MCU EXTI0 Interrupt request service function
  * @param   因为系统必须为上升沿检测，而程序设置为边沿，所以需要在服务函数内部判断当前IO状态，以此来达到上升沿检测的效果
  * @return  None
***/
void EXTI0_ISRQ_Handler(void)
{
    if(P32)	FLAG_EXTISource_0 = 1;
}

/**
  * @name    EXTI1_ISRQ_Handler
  * @brief   MCU EXTI1 Interrupt request service function
  * @param   因为系统必须为上升沿检测，而程序设置为边沿，所以需要在服务函数内部判断当前IO状态，以此来达到上升沿检测的效果
  * @return  None
***/
void EXTI1_ISRQ_Handler(void)
{
    if(P33)	FLAG_EXTISource_1 = 1;
}
/*--------------------------------------------------------
| @Description: Interrupt service function               |
--------------------------------------------------------*/
/*
 * TIMER:
 * void TIMER0_ISRQ_Handler(void)
 * void TIMER1_ISRQ_Handler(void)
 * void TIMER2_ISRQ_Handler(void)
 * void TIMER3_ISRQ_Handler(void)
 * void TIMER4_ISRQ_Handler(void)
 *
 * UART:
 * void UART1_ISRQ_Handler(void)
 * void UART2_ISRQ_Handler(void)
 * void UART3_ISRQ_Handler(void)
 * void UART4_ISRQ_Handler(void)
 *
 * EXTI:
 * void EXTI0_ISRQ_Handler(void)
 * void EXTI1_ISRQ_Handler(void)
 * void EXTI2_ISRQ_Handler(void)
 * void EXTI3_ISRQ_Handler(void)
 *
 * ADC:
 * void ADC_ISRQ_Handler(void)
 *
 * COMP:
 * void COMP_ISRQ_Handler(void)
 *
 * PCA:
 * void PCA_ISRQ_Handler(void)
 *
 * LVD:
 * void LVD_ISRQ_Handler(void)
 *
 * PWM:
 * void PWM_ISRQ_Handler(void)
 * void PWM_ABD_ISRQ_Handler(void)
 *
 */

/*-----------------------------------------------------------------------
|                   END OF FLIE.  (C) COPYRIGHT zeweni                  |
-----------------------------------------------------------------------*/