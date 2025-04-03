/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	main.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-05-08
	*
	*	版本： 		V1.0
	*
	*	说明： 		接入onenet，上传数据和命令控制
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"


//网络协议层
#include "onenet.h"

//网络设备
#include "esp8266.h"

//硬件驱动
#include "delay.h"
#include "usart.h"
#include "beep.h"
#include "AD.h"
#include "PWM.h"
#include "Timer.h"
#include "dht11.h"

//C库
#include <string.h>


#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"mqtts.heclouds.com\",1883\r\n"


/*
************************************************************
*	函数名称：	Hardware_Init
*
*	函数功能：	硬件初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		初始化单片机功能以及外接设备
************************************************************
*/
void Hardware_Init(void)
{
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断控制器分组设置

	Delay_Init();									//systick初始化
	
	Usart1_Init(115200);							//串口1，打印信息用
	
	Usart2_Init(115200);							//串口2，驱动ESP8266用
	
	AD_Init();                                      //温度
    
	// Beep_Init();									//led初始化
    
    PWM_Init();                                      //风扇
    
    Timer_Init();                                      //定时器    

	DHT11_Init();
	
	UsartPrintf(USART_DEBUG, " Hardware init OK\r\n");
	
}

/*
************************************************************
*	函数名称：	main
*
*	函数功能：	
*
*	入口参数：	无
*
*	返回参数：	0
*
*	说明：		
************************************************************
*/
uint16_t Num;
uint16_t temp111,speed,speedtem;
u8 temp;
u8 humi;
volatile uint8_t send_data_flag = 0; // 标志位
int main(void)
{
	unsigned short timeCount = 0;	//发送间隔变量
    unsigned char *dataPtr = NULL;

	Hardware_Init();				//初始化外围硬件
	ESP8266_Init();					//初始化ESP8266
	
	UsartPrintf(USART_DEBUG, "Connect MQTTs Server...\r\n");
	while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
		DelayXms(500);
    UsartPrintf(USART_DEBUG, "Connect MQTTs Server Success\r\n");
	
	while(OneNet_DevLink())			//接入OneNET
		DelayXms(1000);
    
    
    OneNET_Subscribe();
   

	while(1)
	{
        if (send_data_flag) {
            send_data_flag = 0; // 清除标志位
           if(++timeCount >= 5)									//发送间隔5s
            {
				DHT11_Read_Data(&temp,&humi);
				UsartPrintf(USART_DEBUG,"temp: %d ,humi: %d\r\n",temp,humi);
                // temp=AD_GetValue();
                // speed=PWM_TIM_GetCapture1();
                OneNet_SendData();									//发送数据
                timeCount = 0;
                ESP8266_Clear();
                // UsartPrintf(USART_DEBUG,"dayin PWM:%d\r\n",speed);
            }
        }
        dataPtr = ESP8266_GetIPD(0);
            if(dataPtr != NULL)
                OneNet_RevPro(dataPtr);
		 PWM_SetCompare3(speedtem);
        //		DelayMs(10);
	}

}


void TIM2_IRQHandler(void)
{
   
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		
        send_data_flag = 1; // 设置标志位
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}