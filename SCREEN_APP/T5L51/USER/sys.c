#include "sys.h"
#include "task.h"


static T5L_IDATA T5L_VOLATILE u16 delay_tick = 0; //用来实现精确延时的
T5L_XDATA u8 DW_Tx_Buf[10];


//核心寄存器初始化
void sys_cpu_init(void)
{
	EA = 0;
	RS0 = 0;
	RS1 = 0;

	CKCON = 0x00;
	T2CON = 0x70;
	DPC = 0x00;
	PAGESEL = 0x01;
	D_PAGESEL = 0x02; //DATA RAM  0x8000-0xFFFF
	MUX_SEL = 0x00;   //UART2,UART3关闭，WDT关闭
	RAMMODE = 0x00;
	PORTDRV = 0x01;   //驱动强度+/-8mA
	IEN0 = 0x00;      //关闭所有中断
	IEN1 = 0x00;
	IEN2 = 0x00;
	IP0 = 0x00;       //中断优先级默认
	IP1 = 0x00;

	WDT_OFF();      	//关闭开门狗
}


//定时器2初始化,定时间隔为1ms
void sys_timer2_init(void)
{
	T2CON = 0x70;
	TH2 = 0x00;
	TL2 = 0x00;

	TRL2H = 0xBC;	//1ms的定时器
	TRL2L = 0xCD;       

	IEN0 |= 0x20;	//启动定时器2
	TR2 = 0x01;
	EA = 1;
}


//系统初始化
void sys_init(void)
{
	sys_cpu_init();//核心寄存器初始化
	sys_timer2_init();//定时器2初始化
}


//软件大致延时,单位ms
//如果修改了优化等级,那么此函数内部的参数需要重新调试
void sys_delay_about_ms(u16 ms)
{
	u16 i,j;
	for(i=0;i<ms;i++)
			for(j=0;j<3000;j++);    
}


//软件大致延时,单位us
//如果修改了优化等级,那么此函数内部的参数需要重新调试
void sys_delay_about_us(u8 us)
{
	u8 i,j;
	for(i=0;i<us;i++)
			for(j=0;j<5;j++);    
}


//利用定时器2进行精确延时,单位ms
void sys_delay_ms(u16 ms)
{
	delay_tick = ms;
	while(delay_tick);
}


//读DGUS中的VP变量数据
//addr:就是直接传入DGUS中的地址
//buf:缓冲区
//len:读取的字数,一个字等于2个字节
void sys_read_vp(u16 addr,u8* buf,u16 len)
{   
	u8 i; 
	
	i = (u8)(addr&0x01);
	addr >>= 1;
	ADR_H = 0x00;
	ADR_M = (u8)(addr>>8);
	ADR_L = (u8)addr;
	ADR_INC = 0x01;
	RAMMODE = 0xAF;
	while(APP_ACK==0);
	while(len>0)
	{   
		APP_EN=1;
		while(APP_EN==1);
		if((i==0)&&(len>0))   
		{   
			*buf++ = DATA3;
			*buf++ = DATA2;                      
			i = 1;
			len--;	
		}
		if((i==1)&&(len>0))   
		{   
			*buf++ = DATA1;
			*buf++ = DATA0;                      
			i = 0;
			len--;	
		}
	}
	RAMMODE = 0x00;
}


//写DGUS中的VP变量数据
//addr:就是直接传入DGUS中的地址
//buf:缓冲区
//len:被发送数据的字数,一个字等于2个字节
void sys_write_vp(u16 addr,u8* buf,u16 len)
{   
	u8 i;  
	
	i = (u8)(addr&0x01);
	addr >>= 1;
	ADR_H = 0x00;
	ADR_M = (u8)(addr>>8);
	ADR_L = (u8)addr;    
	ADR_INC = 0x01;
	RAMMODE = 0x8F;
	while(APP_ACK==0);
	if(i && len>0)
	{	
		RAMMODE = 0x83;	
		DATA1 = *buf++;		
		DATA0 = *buf++;	
		APP_EN = 1;		
		len--;
	}
	RAMMODE = 0x8F;
	while(len>=2)
	{	
		DATA3 = *buf++;		
		DATA2 = *buf++;
		DATA1 = *buf++;		
		DATA0 = *buf++;
		APP_EN = 1;		
		len -= 2;
	}
	if(len)
	{	
		RAMMODE = 0x8C;
		DATA3 = *buf++;		
		DATA2 = *buf++;
		APP_EN = 1;
	}
	RAMMODE = 0x00;
} 

/* DGUS words are always big-endian. Keil C51 stores multi-byte integers in
 * that order, but SDCC stores them little-endian. Keep the raw byte-buffer
 * API above for protocol commands and use these typed helpers for integers. */
void sys_read_vp_u16(u16 addr, u16 *value)
{
	u8 bytes[2];
	sys_read_vp(addr, bytes, 1);
	*value = ((u16)bytes[0] << 8) | (u16)bytes[1];
}

void sys_write_vp_u16(u16 addr, u16 value)
{
	u8 bytes[2];
	bytes[0] = (u8)(value >> 8);
	bytes[1] = (u8)value;
	sys_write_vp(addr, bytes, 1);
}

void sys_read_vp_u32(u16 addr, u32 *value)
{
	u8 bytes[4];
	sys_read_vp(addr, bytes, 2);
	*value = ((u32)bytes[0] << 24) |
			 ((u32)bytes[1] << 16) |
			 ((u32)bytes[2] << 8) |
			 (u32)bytes[3];
}

void sys_write_vp_u32(u16 addr, u32 value)
{
	u8 bytes[4];
	bytes[0] = (u8)(value >> 24);
	bytes[1] = (u8)(value >> 16);
	bytes[2] = (u8)(value >> 8);
	bytes[3] = (u8)value;
	sys_write_vp(addr, bytes, 2);
}


void write_page(u8 ID)
{
	DW_Tx_Buf[0] = 0x5A;
	DW_Tx_Buf[1] = 0x01;
	DW_Tx_Buf[2] = 0x00;
	DW_Tx_Buf[3] = ID;
	sys_write_vp(0x0084,(u8*)&DW_Tx_Buf,2);
}

void write_beep(u16 Time)
{
	DW_Tx_Buf[0] = Time >> 8;
	DW_Tx_Buf[1] = Time;
	sys_write_vp(0x00A0,(u8*)&DW_Tx_Buf,1);
	
}

//定时器2中断服务程序
void sys_timer2_isr(void) T5L_ISR(5)
{
	TF2=0;//清除定时器2的中断标志位
	
	//精准延时处理
	if(delay_tick)
		delay_tick--;
	
	Task_Tick++;
	
	if(Hat_Time)
		Hat_Time--;
	
	if(Write_Flash > 0)
		Write_Flash--;
}



