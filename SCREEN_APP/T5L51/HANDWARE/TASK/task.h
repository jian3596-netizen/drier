#ifndef __TASK_H__
#define __TASK_H__
#include "sys.h"

//extern xdata u32 Res1,Res2;
extern T5L_XDATA u16 Temp1,Temp2,Temp3;
extern T5L_XDATA u8 Tx_Buf[20];
extern T5L_XDATA u8 Hat_State;
extern T5L_XDATA u8 SW1_State, SW2_State;
extern T5L_XDATA u32 Task_Tick;
extern T5L_XDATA u32 Hat_Time;
extern T5L_XDATA u16 Set_Hat_Time;
extern T5L_XDATA u16 Set_Hat_Temp;
extern T5L_XDATA u16 Set_War_Temp;
extern T5L_XDATA u16 Set_Temp_xs;
extern T5L_XDATA u16 Set_Temp_xs2;

extern T5L_XDATA s32 Write_Flash;
extern T5L_XDATA u16 Set_Hat_Time_old;
extern T5L_XDATA u16 Set_Hat_Temp_old;
extern T5L_XDATA u16 Set_War_Temp_old;
extern T5L_XDATA u16 Set_Temp_xs_old;
extern T5L_XDATA u16 Set_Temp_xs_old2;


void Temp_Init(void);
u16 GET_Temp(u32 Res);
void USART_Send(void);

#endif


