#ifndef _MICK_IO_H_
#define _MICK_IO_H_

#include "stm32f4xx.h"







#define LED1_FLIP  GPIO_Flip_level(GPIOF,GPIO_Pin_13)   // 上位机发送指令 闪烁一次
#define LED2_FLIP  GPIO_Flip_level(GPIOF,GPIO_Pin_14)   // 遥控器接收指令闪烁
#define LED3_FLIP  GPIO_Flip_level(GPIOF,GPIO_Pin_15)    // CAN总线中断 闪烁


 
//mick robot 机器人控制板
void Init_Mick_GPIO(void);

// 读取编码器的编码值
uint8_t Read_Code_Switch(void);

// 读取按键 1 和 2 的按键状态
uint8_t Read_Key(uint8_t channel); 

//读取隔离输入通道的状态
uint8_t Read_Isolated_Input(uint8_t ch);

//设置隔离输出的状态
void Set_Isolated_Output(uint8_t ch, uint8_t out_value);

//读取4个IO状态上传到PC
void Isolated_IO_Upload_Message(void);
#endif
