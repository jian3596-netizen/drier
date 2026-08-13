#ifndef __TASK_H__
#define __TASK_H__
#include "sys.h"
#include <stdio.h>

//extern xdata u32 Res1,Res2;
extern xdata u16 Temp1,Temp2,Temp3;
extern xdata u8 Tx_Buf[20];
extern xdata u8 Hat_State;
extern xdata u8 SW1_State, SW2_State;
extern xdata u32 Task_Tick;
extern xdata u32 Hat_Time;
extern xdata u16 Set_Hat_Time;
extern xdata u16 Set_Hat_Temp;
extern xdata u16 Set_War_Temp;
extern xdata u16 Set_Temp_xs;
extern xdata u16 Set_Temp_xs2;

extern xdata s32 Write_Flash;
extern xdata u16 Set_Hat_Time_old;
extern xdata u16 Set_Hat_Temp_old;
extern xdata u16 Set_War_Temp_old;
extern xdata u16 Set_Temp_xs_old;
extern xdata u16 Set_Temp_xs_old2;


void Temp_Init(void);
u16 GET_Temp(u32 Res);
void USART_Send(void);

#endif


