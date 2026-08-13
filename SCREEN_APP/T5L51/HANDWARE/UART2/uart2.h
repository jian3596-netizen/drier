#ifndef __UART2_H__
#define __UART2_H__
#include "sys.h"


#define UART2_INT_EN							1				//串口中断是否使能
#define UART2_PACKET_OK						0x8000  //是否接受到了完整的数据包
#define UART2_PACKET_LEN					0x7fff	//数据包的长度
#define UART2_PACKET_MAX_LEN			100			//定义了串口包的最大长度,不包含'\n'或"\r\n"结束标记在内的



//变量申明
extern T5L_XDATA T5L_VOLATILE u16 uart2_rx_sta;
extern T5L_XDATA u8  uart2_buf[];

//函数申明
void uart2_init(u32 baud);
void uart2_release_packet(void);
void u2_send_byte(u8 byte);
void u2_send_bytes(u8 *bytes,u16 len);
void uart2_isr(void) T5L_ISR(4);

#endif


