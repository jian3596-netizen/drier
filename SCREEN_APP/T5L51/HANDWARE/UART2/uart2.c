#include "uart2.h"


#if(UART2_INT_EN)
T5L_XDATA T5L_VOLATILE u16 uart2_rx_sta;//bit15用于标志是否已接受到一个完整的数据包,bit[14:0]用于存储当前数据包的长度
T5L_XDATA u8  uart2_buf[UART2_PACKET_MAX_LEN+2];//留2个空字符的位置

//串口2中断服务程序
//发送数据时,必须关闭中断,这里只负责处理接受中断
void uart2_isr(void) T5L_ISR(4)
{
	u8 res;
	
	if(RI0)//是串口接受中断
	{
		RI0 = 0;//清除接受中断标志
		res = SBUF0;//读取串口数据
		
		if(uart2_rx_sta&UART2_PACKET_OK)//接收好的数据还未被处理
			return;
	
		/* Frames contain binary resistance bytes, so 0x0A/0x0D may legally
		 * occur inside their payload and cannot be used as delimiters. Resync
		 * on FA/FB and complete by the frame header:
		 * normal resistance frame = 14 bytes; stop command = 4 bytes. */
		if(uart2_rx_sta == 0)
		{
			if(res == 0xFA || res == 0xFB)
				uart2_buf[uart2_rx_sta++] = res;
		}
		else
		{
			uart2_buf[uart2_rx_sta++] = res;

			if((uart2_rx_sta == 4 && uart2_buf[0] == 0xFB) ||
			   (uart2_rx_sta == 14 && uart2_buf[0] == 0xFA))
			{
				if((uart2_buf[0] == 0xFA && uart2_buf[13] == 0xAF) ||
				   (uart2_buf[0] == 0xFB && uart2_buf[3] == 0xBF))
					uart2_rx_sta |= UART2_PACKET_OK;
				else
					uart2_rx_sta = 0;
			}
			else if(uart2_rx_sta >= UART2_PACKET_MAX_LEN)
			{
				uart2_rx_sta = 0;
			}
		}
		
	}	
}
#endif


//串口2初始化
void uart2_init(u32 baud)
{
	MUX_SEL |= 0x40;//bit6置1表示将uart2接口引出到P0.4和P0.5
	P0MDOUT &= 0xCF;
	P0MDOUT |= 0x10;//设置对应的IO口输出输入
	ADCON = 0x80;//选择SREL0H:L作为波特率发生器
	SCON0 = 0x50;//接受使能和模式设置
	PCON &= 0x7F;//SMOD=0
	//波特率设置,公式为:
	//SMOD=0  SREL0H:L=1024-主频/(64*波特率),SMOD=1	 SREL0H:L=1024-主频/(32*波特率)
	baud = 1024UL-(3225600UL/baud);
	SREL0H = (baud>>8)&0xff;  
	SREL0L = baud&0xff;
	
	#if(UART2_INT_EN)
		ES0 = 1;//中断使能
		EA = 1;
		//xdata变量都得在函数中初始化
		uart2_rx_sta = 0;
	#else
		ES0 = 0;
	#endif

}

void uart2_release_packet(void)
{
	#if(UART2_INT_EN)
		ES0 = 0;
	#endif
	uart2_rx_sta = 0;
	#if(UART2_INT_EN)
		ES0 = 1;
	#endif
}

//发送一个字节
void u2_send_byte(u8 byte)
{
	ES0 = 0;//先关闭串口2中断
	SBUF0 = byte;
	while(!TI0);
	TI0 = 0;
	#if(UART2_INT_EN)
		ES0 = 1;//再打开中断
	#endif
}

//发送数据
void u2_send_bytes(u8 *bytes,u16 len)
{
	u16 i;
	
	ES0 = 0;//先关闭串口2中断
	for(i=0;i<len;i++)
	{
		SBUF0 = bytes[i];
		while(!TI0);
		TI0 = 0;
	}
	#if(UART2_INT_EN)
		ES0 = 1;//再打开中断
	#endif
}


//用uart2串口实现printf函数
char putchar(char c)
{
	u2_send_byte(c);
	
	return c;
}












