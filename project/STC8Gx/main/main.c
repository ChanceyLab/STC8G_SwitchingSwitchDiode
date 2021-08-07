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

#define MAIN_Fosc_Khz 	11059.2				//����ϵͳʱ��Ƶ��(Khz)

#define RELAY_1			P10					//�̵��������������־���ߵ�ƽ����
#define RELAY_2			P11
#define RELAY_3			P16
#define RELAY_4			P17

#define Action_Signal	P37					//Ͷ���źż��IO����

#define	Phase_In_A		P32					//�����ж�IO�Ķ���
#define	Phase_In_B		P33
//�˴�Ϊ�Ĵ�����˿��ض���
#define ET2				0x04
#define ET3				0x20
#define ET4				0x40


u8 		FLAG_Phase_Sequence 	= 0;		//�����־λ��		1--A before B 	2--B before A
u8 		FLAG_Switch_State 		= 0;		//����״̬��־λ��	0--δͶ�� 	1--��Ͷ�� 	Ĭ�ϳ�ʼ״̬ΪδͶ��

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
    //ϵͳ����11.0592MhzƵ������
    //delay_ms(1)ʵ����ʱΪ0.5ms
    //10ΪADC���ֵ5V-1024
    u16 Timer2_Count_Temp = 0;


    STC8x_System_Init();
	
	if(FLAG_Switch_State)					//�Ѿ�Ͷ��״̬
	{
		RELAY_1 = 1;
		RELAY_2 = 0;
		RELAY_3 = 1;
		RELAY_4 = 0;
	}
	else									//�г�״̬
	{
		RELAY_1 = 0;
		RELAY_2 = 0;
		RELAY_3 = 0;
		RELAY_4 = 0;
	}
    //printf("Vin = %d\r\n",ADC_Get_Sample(ADC_Channel_P36_8G1K08_T,ADC_Acc_10Bits));

    /*�̵���24V����ʱ�����
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
	
    /*�ⲿ�ж������жϲ���	EXTI0---B��---P32;EXTI1---A��---P33
    ����˼·��
    T0-1ms�жϵĴ�ǰ���£�������ĳһ�࣬������־λ��
    T0��ʼ������ֱ����⵽��һ��Ϊֹ��
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
    delay_ms(4);							//��һ�����Ϊ��׼��������׼��
    while(1)
    {
        AB_Delat_Time_EN = 1;				//����ʱ�����
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
    NVIC_EXTI0_Init(EXTI_Tri_Edge,NVIC_PR1,DISABLE);//����ⶨ��ɹر�B����ⲿ�ж�
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
  * @param   ������Ŀ�ľ���Ϊ��֪��AB����̵����պ�ʱ���
  * @return  None
***/
void Phase_A_before_B(void)
{
    //��Ͷ��
	//�˴�ʱ��ͼΪ��https://zl279lj20w.feishu.cn/docs/doccnqrbTx3B8OoYRqgz93QZfkg# 8��6��ͼ1
    if(!Action_Signal && !FLAG_Switch_State)
    {
        FLAG_EXTISource_1 = 0;
        while(!FLAG_EXTISource_1);			//�ȴ�A��0���Դ�Ϊԭ��
        
        delay_ms(14);						//K1A(SW4)�պ�ǰ�ȴ�(15-7.6=7.4)ms
        delay_100us();						
        delay_100us();						
        delay_100us();						
        delay_100us();						
        RELAY_4 = 1;						//K1A(SW4)�պ�ָ���
        delay_ms(26);						//K1B(SW2)�պ�ָ���ǰ�ȴ�(13.3)ms
        delay_100us();						
        delay_100us();						
        delay_100us();	
		RELAY_2 = 1;						//K1B(SW2)�պ�ָ���
		delay_ms(3);						//K1A(SW4)�Ͽ�ָ���ǰ�ȴ�1.5ms
        RELAY_4 = 0;						//K1A(SW4)�Ͽ�ָ���
        delay_100us();						//K2A(SW3)�պ�ָ���ǰ�ȴ�0.4ms
        delay_100us();						
        delay_100us();						
        delay_100us();	
		RELAY_3 = 1;						//K2A(SW3)�պ�ָ���
		delay_ms(25);						//K1B(SW2)�Ͽ�ָ���ǰ�ȴ�(13.3-0.4=12.9)ms
        delay_100us();						
        delay_100us();						
        delay_100us();						
        delay_100us();	
		RELAY_2 = 0;						//K1B(SW2)�Ͽ�ָ���
        delay_100us();						//K2B(SW1)�պ�ָ���ǰ�ȴ�0.4ms
        delay_100us();						
        delay_100us();						
        delay_100us();	
		RELAY_1 = 1;						//K2B(SW1)�պ�ָ���
		
		FLAG_Switch_State = 1;					//Ͷ��������ñ�־λ	
    }
    //���г�,�г���Ͷ��ʱ����ȫһ�£���ͬ�Ľ�Ϊ�̵���������ͬ
	//�˴�ʱ��ͼΪ��https://zl279lj20w.feishu.cn/docs/doccnqrbTx3B8OoYRqgz93QZfkg# 8��6��ͼ1
    if(Action_Signal && FLAG_Switch_State)
    {
        FLAG_EXTISource_1 = 0;
        while(!FLAG_EXTISource_1);			//�ȴ�A��0���Դ�Ϊԭ��
        
        delay_ms(14);						//K1A(SW4)�պ�ǰ�ȴ�(15-7.6=7.4)ms
        delay_100us();						
        delay_100us();						
        delay_100us();						
        delay_100us();						
        RELAY_4 = 1;						//K1A(SW4)�պ�ָ���
        delay_ms(26);						//K1B(SW2)�պ�ָ���ǰ�ȴ�(13.3)ms
        delay_100us();						
        delay_100us();						
        delay_100us();	
		RELAY_2 = 1;						//K1B(SW2)�պ�ָ���
		delay_ms(3);						//K1A(SW4)�Ͽ�ָ���ǰ�ȴ�1.5ms
        RELAY_4 = 0;						//K1A(SW4)�Ͽ�ָ���
        delay_100us();						//K2A(SW3)�Ͽ�ָ���ǰ�ȴ�0.4ms
        delay_100us();						
        delay_100us();						
        delay_100us();	
		RELAY_3 = 0;						//K2A(SW3)�Ͽ�ָ���
		delay_ms(25);						//K1B(SW2)�Ͽ�ָ���ǰ�ȴ�(13.3-0.4=12.9)ms
        delay_100us();						
        delay_100us();						
        delay_100us();						
        delay_100us();	
		RELAY_2 = 0;						//K1B(SW2)�Ͽ�ָ���
        delay_100us();						//K2B(SW1)�Ͽ�ָ���ǰ�ȴ�0.4ms
        delay_100us();						
        delay_100us();						
        delay_100us();	
		RELAY_1 = 0;						//K2B(SW1)�Ͽ�ָ���
		
		FLAG_Switch_State = 0;					//�г�������ñ�־λ	
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
    //��Ͷ��
	//�˴�ʱ��ͼΪ��https://zl279lj20w.feishu.cn/docs/doccnqrbTx3B8OoYRqgz93QZfkg# 8��6��ͼ2
    if(!Action_Signal && !FLAG_Switch_State)
    {
        FLAG_EXTISource_1 = 0;
        while(!FLAG_EXTISource_1);			//�ȴ�A��0���Դ�Ϊԭ��
		
        delay_ms(14);						//K1A(SW4)�պ�ǰ�ȴ�(15-7.6=7.4)ms
        delay_100us();						
        delay_100us();						
        delay_100us();						
        delay_100us();						
        RELAY_4 = 1;						//K1A(SW4)�պ�ָ���
		delay_ms(13);						//K1B(SW2)�պ�ָ���ǰ��(20/3=6.7)ms
        delay_100us();						
        delay_100us();						
		RELAY_2 = 1;						//K1B(SW2)�պ�ָ���
		delay_ms(16);						//K1A(SW4)�պ�ָ���ǰ��(5.9+2.2=8.1)ms
        delay_100us();						
		RELAY_4 = 1;						//K1A(SW4)�պ�ָ���
        delay_100us();						//K2A(SW3)�պ�ָ���ǰ��(0.4)ms				
        delay_100us();						
        delay_100us();						
        delay_100us();						
		RELAY_3 = 1;						//K2A(SW3)�պ�ָ���
		delay_ms(8);						//K2B(SW1)�պ�ָ���ǰ�ȴ�(6.7-2.4=4.3)ms
        delay_100us();						
        delay_100us();						
        delay_100us();						
		RELAY_1 = 1;						//K2B(SW1)�պ�ָ���
        delay_100us();						//K1B(SW2)�պ�ָ���ǰ��(0.4)ms				
        delay_100us();						
        delay_100us();						
        delay_100us();						
		RELAY_2 = 1;						//K1B(SW2)�պ�ָ���
    
		FLAG_Switch_State = 1;				//Ͷ��������ñ�־λ	
	}
    //���г�,�г���Ͷ��ʱ����ȫһ�£���ͬ�Ľ�Ϊ�̵���������ͬ
    if(Action_Signal && FLAG_Switch_State)
    {
        FLAG_EXTISource_1 = 0;
        while(!FLAG_EXTISource_1);			//�ȴ�A��0���Դ�Ϊԭ��
		
        delay_ms(14);						//K1A(SW4)�պ�ǰ�ȴ�(15-7.6=7.4)ms
        delay_100us();						
        delay_100us();						
        delay_100us();						
        delay_100us();						
        RELAY_4 = 1;						//K1A(SW4)�պ�ָ���
		delay_ms(13);						//K1B(SW2)�պ�ָ���ǰ��(20/3=6.7)ms
        delay_100us();						
        delay_100us();						
		RELAY_2 = 1;						//K1B(SW2)�պ�ָ���
		delay_ms(16);						//K1A(SW4)�պ�ָ���ǰ��(5.9+2.2=8.1)ms
        delay_100us();						
		RELAY_4 = 1;						//K1A(SW4)�պ�ָ���
        delay_100us();						//K2A(SW3)�պ�ָ���ǰ��(0.4)ms				
        delay_100us();						
        delay_100us();						
        delay_100us();						
		RELAY_3 = 0;						//K2A(SW3)�պ�ָ���
		delay_ms(8);						//K2B(SW1)�պ�ָ���ǰ�ȴ�(6.7-2.4=4.3)ms
        delay_100us();						
        delay_100us();						
        delay_100us();						
		RELAY_1 = 0;						//K2B(SW1)�պ�ָ���
        delay_100us();						//K1B(SW2)�պ�ָ���ǰ��(0.4)ms				
        delay_100us();						
        delay_100us();						
        delay_100us();						
		RELAY_2 = 1;						//K1B(SW2)�պ�ָ���
    
		FLAG_Switch_State = 0;				//Ͷ��������ñ�־λ	
    }
}

/*-----------------------------------------------------------------------
|                   END OF FLIE.  (C) COPYRIGHT zeweni                  |
-----------------------------------------------------------------------*/


