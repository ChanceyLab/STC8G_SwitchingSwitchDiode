/*-----------------------------------------------------------------------
|                            FILE DESCRIPTION                           |
-----------------------------------------------------------------------*/
/*----------------------------------------------------------------------
  - File name     : main.c
  - Author        : zeweni
  - Update date   : 2020.01.11
  -	Copyright(C)  : 2020-2021 zeweni. All rights reserved.
-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------
|                               INCLUDES                                |
-----------------------------------------------------------------------*/
#include "main.h"
#include "intrins.h"
/*-----------------------------------------------------------------------
|                                 DATA                                  |
-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
|                               FUNCTION                                |
-----------------------------------------------------------------------*/

#define MAIN_Fosc_Khz 	11059.2				//定义系统时钟频率(Khz)

#define RELAY_1			P10					//继电器定义见开发日志，高电平吸合
#define RELAY_2			P11
#define RELAY_3			P16
#define RELAY_4			P17

#define Action_Signal	P37					//投切信号检测IO定义

#define	Phase_In_A		P32					//相序判断IO的定义
#define	Phase_In_B		P33
//此处为寄存器与端口重定义
#define ET2				0x04
#define ET3				0x20
#define ET4				0x40


u8 		FLAG_Phase_Sequence 	= 0;		//相序标志位：		1--A before B 	2--B before A
u8 		FLAG_Switch_State 		= 0;		//开关状态标志位：	0--未投入 	1--已投入 	默认初始状态为未投出

extern 	u8 	G_KEY_VALUE;
extern 	u8 	Timer0_ISRQ_Length_10;
extern 	u8 	FLAG_EXTISource_0;
extern 	u8 	FLAG_EXTISource_1;
extern 	u16 Timer2_Count;
extern 	u8 	AB_Delat_Time_ms;
extern 	u8 	AB_Delat_Time_EN;

void Phase_A_before_B(void);
void Phase_B_before_A(void);
void Switch_Put_In(void);
void Switch_Cut_out(void);

/**
  * @name    delay_100us
  * @brief   delay program,system in 11.0592Mhz
  * @param   None
  * @return  None
***/
void delay_100us(void)
{
    u8 a,b;
    for(a = 10; a > 0; a --)
    {
        for(b = 16; b > 0; b --);
    }
}


/**
  * @name    main
  * @brief   main program
  * @param   None
  * @return  None
***/
void main(void)
{
    //系统采用11.0592Mhz频率运行
    //delay_ms(1)实际延时为0.5ms
    //10为ADC最大值5V-1024
    u16 Timer2_Count_Temp = 0;


    STC8x_System_Init();
	
	if(FLAG_Switch_State)					//已经投入状态
	{
		RELAY_1 = 1;
		RELAY_2 = 0;
		RELAY_3 = 1;
		RELAY_4 = 0;
	}
	else									//切出状态
	{
		RELAY_1 = 0;
		RELAY_2 = 0;
		RELAY_3 = 0;
		RELAY_4 = 0;
	}
    //printf("Vin = %d\r\n",ADC_Get_Sample(ADC_Channel_P36_8G1K08_T,ADC_Acc_10Bits));

    /*继电器24V动作时间测试
    while(1)
    {
    	switch(G_KEY_VALUE % 2)
    	{
    		case 0:
    			RELAY_? = 0;
    			break;
    		case 1:
    			RELAY_? = 1;
    			break;
    	}
    }*/
	
    /*外部中断相序判断测试	EXTI0---B相---P32;EXTI1---A相---P33
    程序思路：
    T0-1ms中断的大前提下，如果检测某一相，开启标志位，
    T0开始计数，直到检测到另一相为止。
    printf("SYS INIT DONE!\r\n");
    while(1)
    {
    	if((FLAG_EXTISource_0 + FLAG_EXTISource_1) == 2)
        {
            printf("Delat Time is %bd ms\r\n",AB_Delat_Time_ms);
            FLAG_EXTISource_0 = 0;
    		FLAG_EXTISource_1 = 0;
    		AB_Delat_Time_ms = 0;

    		while(1);
        }
    }
    */
    delay_ms(4);							//等一会儿，为精准测相序做准备
    while(1)
    {
        AB_Delat_Time_EN = 1;				//开启时间测量
        FLAG_EXTISource_1 = 0;
        while(!FLAG_EXTISource_1);
        FLAG_EXTISource_0 = 0;
        while(!FLAG_EXTISource_0);
        AB_Delat_Time_EN = 0;
        //printf("%bd ms\r\n",AB_Delat_Time_ms);
        if(AB_Delat_Time_ms < 10)
        {
            FLAG_Phase_Sequence = 1;		//A before B 
			printf("A before B\r\n");
            break;
        }
        else if(AB_Delat_Time_ms < 20)
        {
            FLAG_Phase_Sequence = 2;		//B before A
			printf("B before A\r\n");
			break;
        }
    }
    NVIC_EXTI0_Init(EXTI_Tri_Edge,NVIC_PR1,DISABLE);//相序测定完成关闭B相的外部中断
	while(1)
	{
		if(FLAG_Phase_Sequence == 1)
		{
			Phase_A_before_B();
		}
		if(FLAG_Phase_Sequence == 2)
		{
			Phase_B_before_A();
		}
	}
}



/**
  * @name    Phase_A_before_B
  * @brief   Phase program
  * @param   测相序目的就是为了知道AB两相继电器闭合时间差
  * @return  None
***/
void Phase_A_before_B(void)
{
    //该投入
	//此处时序图为：https://zl279lj20w.feishu.cn/docs/doccnqrbTx3B8OoYRqgz93QZfkg# 8月6日图1
    if(!Action_Signal && !FLAG_Switch_State)
    {
        FLAG_EXTISource_1 = 0;
        while(!FLAG_EXTISource_1);			//等待A相0点以此为原点
        
        delay_ms(14);						//K1A(SW4)闭合前等待(15-7.6=7.4)ms
        delay_100us();						
        delay_100us();						
        delay_100us();						
        delay_100us();						
        RELAY_4 = 1;						//K1A(SW4)闭合指令发出
        delay_ms(26);						//K1B(SW2)闭合指令发出前等待(13.3)ms
        delay_100us();						
        delay_100us();						
        delay_100us();	
		RELAY_2 = 1;						//K1B(SW2)闭合指令发出
		delay_ms(3);						//K1A(SW4)断开指令发出前等待1.5ms
        RELAY_4 = 0;						//K1A(SW4)断开指令发出
        delay_100us();						//K2A(SW3)闭合指令发出前等待0.4ms
        delay_100us();						
        delay_100us();						
        delay_100us();	
		RELAY_3 = 1;						//K2A(SW3)闭合指令发出
		delay_ms(25);						//K1B(SW2)断开指令发出前等待(13.3-0.4=12.9)ms
        delay_100us();						
        delay_100us();						
        delay_100us();						
        delay_100us();	
		RELAY_2 = 0;						//K1B(SW2)断开指令发出
        delay_100us();						//K2B(SW1)闭合指令发出前等待0.4ms
        delay_100us();						
        delay_100us();						
        delay_100us();	
		RELAY_1 = 1;						//K2B(SW1)闭合指令发出
		
		FLAG_Switch_State = 1;					//投入完成设置标志位	
    }
    //该切出,切出与投入时序完全一致，不同的仅为继电器动作不同
	//此处时序图为：https://zl279lj20w.feishu.cn/docs/doccnqrbTx3B8OoYRqgz93QZfkg# 8月6日图1
    if(Action_Signal && FLAG_Switch_State)
    {
        FLAG_EXTISource_1 = 0;
        while(!FLAG_EXTISource_1);			//等待A相0点以此为原点
        
        delay_ms(14);						//K1A(SW4)闭合前等待(15-7.6=7.4)ms
        delay_100us();						
        delay_100us();						
        delay_100us();						
        delay_100us();						
        RELAY_4 = 1;						//K1A(SW4)闭合指令发出
        delay_ms(26);						//K1B(SW2)闭合指令发出前等待(13.3)ms
        delay_100us();						
        delay_100us();						
        delay_100us();	
		RELAY_2 = 1;						//K1B(SW2)闭合指令发出
		delay_ms(3);						//K1A(SW4)断开指令发出前等待1.5ms
        RELAY_4 = 0;						//K1A(SW4)断开指令发出
        delay_100us();						//K2A(SW3)断开指令发出前等待0.4ms
        delay_100us();						
        delay_100us();						
        delay_100us();	
		RELAY_3 = 0;						//K2A(SW3)断开指令发出
		delay_ms(25);						//K1B(SW2)断开指令发出前等待(13.3-0.4=12.9)ms
        delay_100us();						
        delay_100us();						
        delay_100us();						
        delay_100us();	
		RELAY_2 = 0;						//K1B(SW2)断开指令发出
        delay_100us();						//K2B(SW1)断开指令发出前等待0.4ms
        delay_100us();						
        delay_100us();						
        delay_100us();	
		RELAY_1 = 0;						//K2B(SW1)断开指令发出
		
		FLAG_Switch_State = 0;					//切出完成设置标志位	
    }
}

/**
  * @name    Phase_B_before_A
  * @brief   Phase program
  * @param   None
  * @return  None
***/
void Phase_B_before_A(void)
{
    //该投入
	//此处时序图为：https://zl279lj20w.feishu.cn/docs/doccnqrbTx3B8OoYRqgz93QZfkg# 8月6日图2
    if(!Action_Signal && !FLAG_Switch_State)
    {
        FLAG_EXTISource_1 = 0;
        while(!FLAG_EXTISource_1);			//等待A相0点以此为原点
		
        delay_ms(14);						//K1A(SW4)闭合前等待(15-7.6=7.4)ms
        delay_100us();						
        delay_100us();						
        delay_100us();						
        delay_100us();						
        RELAY_4 = 1;						//K1A(SW4)闭合指令发出
		delay_ms(13);						//K1B(SW2)闭合指令发出前等(20/3=6.7)ms
        delay_100us();						
        delay_100us();						
		RELAY_2 = 1;						//K1B(SW2)闭合指令发出
		delay_ms(16);						//K1A(SW4)闭合指令发出前等(5.9+2.2=8.1)ms
        delay_100us();						
		RELAY_4 = 1;						//K1A(SW4)闭合指令发出
        delay_100us();						//K2A(SW3)闭合指令发出前等(0.4)ms				
        delay_100us();						
        delay_100us();						
        delay_100us();						
		RELAY_3 = 1;						//K2A(SW3)闭合指令发出
		delay_ms(8);						//K2B(SW1)闭合指令发出前等待(6.7-2.4=4.3)ms
        delay_100us();						
        delay_100us();						
        delay_100us();						
		RELAY_1 = 1;						//K2B(SW1)闭合指令发出
        delay_100us();						//K1B(SW2)闭合指令发出前等(0.4)ms				
        delay_100us();						
        delay_100us();						
        delay_100us();						
		RELAY_2 = 1;						//K1B(SW2)闭合指令发出
    
		FLAG_Switch_State = 1;				//投入完成设置标志位	
	}
    //该切出,切出与投入时序完全一致，不同的仅为继电器动作不同
    if(Action_Signal && FLAG_Switch_State)
    {
        FLAG_EXTISource_1 = 0;
        while(!FLAG_EXTISource_1);			//等待A相0点以此为原点
		
        delay_ms(14);						//K1A(SW4)闭合前等待(15-7.6=7.4)ms
        delay_100us();						
        delay_100us();						
        delay_100us();						
        delay_100us();						
        RELAY_4 = 1;						//K1A(SW4)闭合指令发出
		delay_ms(13);						//K1B(SW2)闭合指令发出前等(20/3=6.7)ms
        delay_100us();						
        delay_100us();						
		RELAY_2 = 1;						//K1B(SW2)闭合指令发出
		delay_ms(16);						//K1A(SW4)闭合指令发出前等(5.9+2.2=8.1)ms
        delay_100us();						
		RELAY_4 = 1;						//K1A(SW4)闭合指令发出
        delay_100us();						//K2A(SW3)闭合指令发出前等(0.4)ms				
        delay_100us();						
        delay_100us();						
        delay_100us();						
		RELAY_3 = 0;						//K2A(SW3)闭合指令发出
		delay_ms(8);						//K2B(SW1)闭合指令发出前等待(6.7-2.4=4.3)ms
        delay_100us();						
        delay_100us();						
        delay_100us();						
		RELAY_1 = 0;						//K2B(SW1)闭合指令发出
        delay_100us();						//K1B(SW2)闭合指令发出前等(0.4)ms				
        delay_100us();						
        delay_100us();						
        delay_100us();						
		RELAY_2 = 1;						//K1B(SW2)闭合指令发出
    
		FLAG_Switch_State = 0;				//投入完成设置标志位	
    }
}

/*-----------------------------------------------------------------------
|                   END OF FLIE.  (C) COPYRIGHT zeweni                  |
-----------------------------------------------------------------------*/


