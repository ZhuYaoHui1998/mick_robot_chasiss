#include "stm32f10x.h"
#include "string.h"
#include "stdlib.h"
#include "bsp_uart.h"

#include "DBUS.h" 
#include "DJI_Motor.h"

 
uint32_t ch1_offset_sum=0,ch2_offset_sum=0,ch3_offset_sum=0,ch4_offset_sum=0;
uint32_t rc_counter=0;

rc_info_t rc; // 这里将DBUS和SBUS共用一个结构体
extern uint8_t Code_Switch_Value;
extern uint8_t USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节
/***************************************************************************
* @brief       DBUS串口接收回调函数 
* @param[out]  rc:   转换为每个通道的数据
* @param[in]   pData: 输入长度为18字节的数据
* @retval 
* @maker    crp
* @date 2019-9-8
***************************************************************************/
char RC_Callback_Handler(uint8_t *pData)
{
	unsigned char i=0;
	if(pData == NULL)
	{
		return 0;
	}
	
	if(rc.type == 1) // 大疆DBUS协议
	{
		rc.ch1 = ((int16_t)pData[0] | ((int16_t)pData[1] << 8)) & 0x07FF;
		rc.ch2 = (((int16_t)pData[1] >> 3) | ((int16_t)pData[2] << 5))	& 0x07FF;
		rc.ch3 = (((int16_t)pData[2] >> 6) | ((int16_t)pData[3] << 2) |((int16_t)pData[4] << 10)) & 0x07FF;
		rc.ch4 = (((int16_t)pData[4] >> 1) | ((int16_t)pData[5]<<7)) &0x07FF;
		rc.sw1 = ((pData[5] >> 4) & 0x000C) >> 2;
		rc.sw2 = ((pData[5] >> 4) & 0x0003);
		rc.x = ((int16_t)pData[6]) | ((int16_t)pData[7] << 8);
		rc.y = ((int16_t)pData[8]) | ((int16_t)pData[9] << 8);
		rc.z = ((int16_t)pData[10]) | ((int16_t)pData[11] << 8);
		rc.press_l = pData[12];
		rc.press_r = pData[13];
		rc.v = ((int16_t)pData[14]);// | ((int16_t)pData[15] << 8);
	}
	else if(rc.type == 2) // 航模SBUS协议
	{
				// SBUS解码
		if(pData[0] == 0x0f && pData[USART_REC_LEN-1]==0 && pData[USART_REC_LEN-2]==0)
		{
			i=0;
		}
		else //DMA是固定长度接收，如果第一个数据不是0x0f那么，后面所有的数据都会错位，因此在这个地方复位
		{	
			UART_send_string(USART2,"SBUS fram head != 0x0f, reset MCU.\n");			
			__set_FAULTMASK(1); // 关闭所有中断
			NVIC_SystemReset();// 复位
		}
//		else  
//		{
//			for(i=2;i<USART_REC_LEN;i++)
//			{
//				if(pData[i-2] == 0x00 &&pData[i-1] == 0x00 && pData[i] == 0x0f)
//				{
//					break;
//				}
//			}
//		}
//		if(i==USART_REC_LEN)
//		{
//				return 0;
//		}
		rc.ch1 = ((int16_t)pData[(i+1)%USART_REC_LEN] | ((int16_t)pData[(i+2)%USART_REC_LEN] << 8)) & 0x07FF;
		rc.ch2 = (((int16_t)pData[(i+2)%USART_REC_LEN] >> 3) | ((int16_t)pData[(i+3)%USART_REC_LEN] << 5))	& 0x07FF;
		rc.ch3 = (((int16_t)pData[(i+3)%USART_REC_LEN] >> 6) | ((int16_t)pData[(i+4)%USART_REC_LEN] << 2) |((int16_t)pData[(i+5)%USART_REC_LEN] << 10)) & 0x07FF;
		rc.ch4 = (((int16_t)pData[(i+5)%USART_REC_LEN] >> 1) | ((int16_t)pData[(i+6)%USART_REC_LEN]<<7)) &0x07FF;
		rc.ch5 = ((int16_t)pData[(i+6)%USART_REC_LEN] >> 4 | ((int16_t)pData[(i+7)%USART_REC_LEN] << 4)) & 0x07FF;
		rc.ch6 = (((int16_t)pData[(i+7)%USART_REC_LEN] >> 7) | ((int16_t)pData[(i+8)%USART_REC_LEN] << 1) |((int16_t)pData[(i+9)%USART_REC_LEN] << 9))	& 0x07FF;
		rc.ch7 = (((int16_t)pData[(i+9)%USART_REC_LEN] >> 2) | ((int16_t)pData[(i+10)%USART_REC_LEN] << 6)) & 0x07FF;
		rc.ch8 = (((int16_t)pData[(i+10)%USART_REC_LEN] >> 5) | ((int16_t)pData[(i+11)%USART_REC_LEN]<<3)) &0x07FF;

		rc.ch9 = ((int16_t)pData[(i+12)%USART_REC_LEN] | ((int16_t)pData[(i+13)%USART_REC_LEN] << 8)) & 0x07FF;
		rc.ch10 = (((int16_t)pData[(i+13)%USART_REC_LEN] >> 3) | ((int16_t)pData[(i+14)%USART_REC_LEN] << 5))	& 0x07FF;
		rc.ch11 = (((int16_t)pData[(i+14)%USART_REC_LEN] >> 6) | ((int16_t)pData[(i+15)%USART_REC_LEN] << 2) |((int16_t)pData[(i+16)%USART_REC_LEN] << 10)) & 0x07FF;
		rc.ch12 = (((int16_t)pData[(i+16)%USART_REC_LEN] >> 1) | ((int16_t)pData[(i+17)%USART_REC_LEN]<<7)) &0x07FF;
		rc.ch13 = ((int16_t)pData[(i+17)%USART_REC_LEN] >> 4 | ((int16_t)pData[(i+18)%USART_REC_LEN] << 4)) & 0x07FF;
		rc.ch14 = (((int16_t)pData[(i+18)%USART_REC_LEN] >> 7) | ((int16_t)pData[(i+19)%USART_REC_LEN] << 1) |((int16_t)pData[(i+20)%USART_REC_LEN] << 9))	& 0x07FF;
		rc.ch15 = (((int16_t)pData[(i+20)%USART_REC_LEN] >> 2) | ((int16_t)pData[(i+21)%USART_REC_LEN] << 6)) & 0x07FF;
		rc.ch16 = (((int16_t)pData[(i+21)%USART_REC_LEN] >> 5) | ((int16_t)pData[(i+22)%USART_REC_LEN]<<3)) &0x07FF;

		if(rc.ch7>1500) 		rc.sw1 = 3; //把通道5-6进行量化到 1-3
		else if(rc.ch7<500)  	rc.sw1 = 1;
		else            		rc.sw1 = 2;
		
		if(rc.ch5>1500) 		rc.sw2 = 2;  // 与DJI遥控器保持一致
		else if(rc.ch5<500)		rc.sw2 = 1;
		else 					rc.sw2 = 3;
		
		rc.update =0x01;
	}
	else
	{
		rc.update =0x01;
		rc.available =0x00;
	}
	 
	
	if(rc.rc_state == DBUS_INIT) 
	{
		UART_send_string(USART2,"RC Init ");
		rc.available =0x00;
		if(RC_Offset_Init())
		{
			rc.available =0x02; //初始化成功
			rc.rc_state = DBUS_RUN;
			UART_send_string(USART2,"successful!\n");
		}
		else
		{
			rc.available =0x01; //计算offset中
			rc.rc_state = DBUS_INIT; //初始化失败
			UART_send_string(USART2,"failed..\n");
		}
	}
	else
	{
		if((rc.ch1 > 2500) || (rc.ch1<100) 
			|| (rc.ch2 > 2500) || (rc.ch2<100)
			|| (rc.ch3 > 2500) || (rc.ch3<100)
			|| (rc.sw1 > 3) || (rc.sw1<1)
			|| (rc.sw2 > 3) || (rc.sw2<1))
		{
			UART_send_string(USART2,"ERROR\n");
			rc.available =0x03; // 初始化以后出现了异常数据
			return 0;
		}
		else
		{
			rc.available =0x0f;
			RC_Routing();
		}
		rc.cnt =rc.cnt +1;	
	}
	
	return 1;
}
/***************************************************************************
* @brief       成功接收到一帧DBUS数据以后调用该函数设定PID控制的目标值
* @retval 
* @maker    crp
* @date 2019-9-8
****************************************************************************/
void RC_Routing(void)
{
	float RC_K = 0;
	int err_ch1 = 0;
	int err_ch2 = 0;
	int err_ch3 = 0;
	if((rc.sw1 !=1) && (rc.available)) //使能遥控器模式
	{
		if(rc.sw2 ==1) //1 档模式 最大1m/s
		{
			RC_K=0.00152;
		}
		else if(rc.sw2 ==3) //2 档模式 最大2m/s
		{
			RC_K = 0.00304;
		}
		else if(rc.sw2 ==2) //3 档模式 最大3.5m/s
		{
			RC_K = 0.0053;
		}
		else
			RC_K = 0.0;
		
		// 设置死区
		err_ch1 = rc.ch1-rc.ch1_offset;
		err_ch2 = rc.ch2-rc.ch2_offset;
		err_ch3 = rc.ch3-rc.ch3_offset;
		
		if(abs(err_ch1) < 50)
				err_ch1 =0;
		if(abs(err_ch2) < 50)
				err_ch2 = 0;
		if(abs(err_ch3) < 50)
				err_ch3 = 0;
		
		if((Code_Switch_Value & 0x03) == 0x00) // 差速模型
		{
			DiffX4_Wheel_Speed_Model(err_ch2*RC_K,err_ch1*RC_K);
		}
		else if((Code_Switch_Value & 0x03) == 0x01) // 麦克纳姆轮模型
		{
			Mecanum_Wheel_Speed_Model(err_ch2*RC_K,-err_ch1*RC_K,-err_ch3*RC_K);
		}
		else if((Code_Switch_Value & 0x03) == 0x02)// 4WS4WD模型
		{
			; 
		}
		else if((Code_Switch_Value & 0x03) == 0x03)// 阿卡曼模型
		{
			; 
		}
		else ;
		
	}
}

/***************************************************************************
* @brief       遥控器初始化函数 采集10次数据求取平均值
* @param[out]  校准完成返回1 否则返回0
* @param[in]    
* @retval 
* @maker    crp
* @date 2019-9-8
****************************************************************************/
char RC_Offset_Init(void)
{
	if((rc.ch1>900) && (rc.ch1<1100))
		if((rc.ch2>900) && (rc.ch2<1100))
			if((rc.ch3>900) && (rc.ch3<1100))
				if((rc.ch4>900) && (rc.ch4<1100))
				{
						ch1_offset_sum+=rc.ch1;
						ch2_offset_sum+=rc.ch2;
						ch3_offset_sum+=rc.ch3;
						ch4_offset_sum+=rc.ch4;
						rc_counter++;
				}
	// 求10次数据的平均值
	if(rc_counter>10)
	{
	  ch1_offset_sum = ch1_offset_sum/rc_counter;
		ch2_offset_sum = ch2_offset_sum/rc_counter;
		ch3_offset_sum = ch3_offset_sum/rc_counter;
		ch4_offset_sum = ch4_offset_sum/rc_counter;
		
		rc.ch1_offset =ch1_offset_sum;
		rc.ch2_offset =ch2_offset_sum;
		rc.ch3_offset =ch3_offset_sum;
		rc.ch4_offset =ch4_offset_sum;
		
		//calibration failed 通常情况这个零位值不会是0
		if((rc.ch1_offset ==0) || (rc.ch2_offset ==0) || (rc.ch3_offset ==0) || (rc.ch4_offset ==0))
		{
			rc.available =0x00; 
			rc_counter=0;
			ch1_offset_sum = 0;
			ch2_offset_sum = 0;
			ch3_offset_sum = 0;
			ch4_offset_sum = 0;
			return 0;
		}
		else
		{
			rc.available =0x01; 
			rc.cnt =rc_counter;
			return 1;
		}
	}
	
	rc.available =0x00;
	rc.cnt =rc_counter;
	
	return 0;
}

/***************************************************************************
* @brief       调试遥控器，打印数据
* @retval 
* @maker    crp
* @date 2019-9-8
****************************************************************************/
void RC_Debug_Message(void)
{
//	unsigned char i=0;
	if(rc.type == 1) // 大疆DBUS协议
	{
		UART_send_string(USART2,"DBUS:  ch1:");UART_send_data(USART2,rc.ch1);UART_send_char(USART2,'\t');		
		UART_send_string(USART2,"ch2:");UART_send_data(USART2,rc.ch2);UART_send_char(USART2,'\t');	
		UART_send_string(USART2,"ch3:");UART_send_data(USART2,rc.ch3);UART_send_char(USART2,'\t');	
		UART_send_string(USART2,"ch4:");UART_send_data(USART2,rc.ch4);UART_send_char(USART2,'\t');	
		UART_send_string(USART2,"sw1:");UART_send_data(USART2,rc.sw1);UART_send_char(USART2,'\t');	
		UART_send_string(USART2,"sw2:");UART_send_data(USART2,rc.sw2);UART_send_char(USART2,'\n');	
	}
	else if(rc.type == 2) // 航模SBUS协议
	{
		UART_send_string(USART2,"SBUS:  ch1:");UART_send_data(USART2,rc.ch1);UART_send_char(USART2,'\t');		
		UART_send_data(USART2,rc.ch2);UART_send_char(USART2,'\t');	
		UART_send_data(USART2,rc.ch3);UART_send_char(USART2,'\t');	
		UART_send_data(USART2,rc.ch4);UART_send_char(USART2,'\t');	
		UART_send_data(USART2,rc.ch5);UART_send_char(USART2,'\t');	
		UART_send_data(USART2,rc.ch6);UART_send_char(USART2,'\t');	
		UART_send_data(USART2,rc.ch7);UART_send_char(USART2,'\t');	
		UART_send_data(USART2,rc.ch8);UART_send_char(USART2,'\t');	
		UART_send_string(USART2,"sw1:");UART_send_data(USART2,rc.sw1);UART_send_char(USART2,'\t');	
		UART_send_string(USART2,"sw2:");UART_send_data(USART2,rc.sw2);UART_send_char(USART2,'\n');	
//		UART_send_data(USART2,rc.ch9);UART_send_char(USART2,'\t');	
//		UART_send_data(USART2,rc.ch10);UART_send_char(USART2,'\t');	
//		UART_send_data(USART2,rc.ch11);UART_send_char(USART2,'\t');	
//		UART_send_data(USART2,rc.ch12);UART_send_char(USART2,'\t');	
//		UART_send_data(USART2,rc.ch13);UART_send_char(USART2,'\t');	
//		UART_send_data(USART2,rc.ch14);UART_send_char(USART2,'\t');	
//		UART_send_data(USART2,rc.ch15);UART_send_char(USART2,'\t');	
//		UART_send_data(USART2,rc.ch16);
		UART_send_char(USART2,'\n');	
		
	}
 
//	UART_send_string(USART2,"cnt:");UART_send_data(USART2,rc.cnt);UART_send_char(USART2,'\t');
//	UART_send_string(USART2,"available:");UART_send_data(USART2,rc.available);UART_send_char(USART2,'\t');
//	UART_send_string(USART2,"ch1_offset:");UART_send_data(USART2,rc.ch1_offset);UART_send_char(USART2,'\t');		
//	UART_send_string(USART2,"ch2_offset:");UART_send_data(USART2,rc.ch2_offset);UART_send_char(USART2,'\t');	
//	UART_send_string(USART2,"ch3_offset:");UART_send_data(USART2,rc.ch3_offset);UART_send_char(USART2,'\t');	
//	UART_send_string(USART2,"ch4_offset:");UART_send_data(USART2,rc.ch4_offset);UART_send_char(USART2,'\n');	
}

/***************************************************************************
* @brief       DBUS上传信息到PC上
* @retval 
* @maker    crp
* @date 2019-9-8
****************************************************************************/

void RC_Upload_Message(void)
{
	unsigned char senddata[50];
	unsigned char i=0,j=0;	
	unsigned char cmd=0x03;	
	unsigned int sum=0x00;	
	senddata[i++]=0xAE;
	senddata[i++]=0xEA;
	senddata[i++]=0x00;
	senddata[i++]=cmd;
	senddata[i++]=(rc.cnt>>24);
	senddata[i++]=(rc.cnt>>16);
	senddata[i++]=(rc.cnt>>8);
	senddata[i++]=(rc.cnt);
	senddata[i++]=rc.ch1>>8;
	senddata[i++]=rc.ch1;
	senddata[i++]=rc.ch2>>8;
	senddata[i++]=rc.ch2;
	senddata[i++]=rc.ch3>>8;
	senddata[i++]=rc.ch3;
	senddata[i++]=rc.ch4>>8;
	senddata[i++]=rc.ch4;
	senddata[i++]=rc.sw1;
	senddata[i++]=rc.sw2;
	senddata[i++]=rc.type;
	senddata[i++]=rc.rc_state;
	senddata[i++]=0x00; //保留字
	for(j=2;j<i;j++)
		sum+=senddata[j];
	senddata[i++]=sum;
	senddata[2]=i-2; //数据长度
	senddata[i++]=0xEF;
	senddata[i++]=0xFE;
	senddata[i++]='\0';
	UART_send_buffer(USART2,senddata,i);
 
}
