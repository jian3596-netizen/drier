#include "sys.h"
#include "uart2.h"
#include "task.h"
#include "nor_flash.h"

xdata u16 val;

void main(void)
{
	u16 len;
	u16 btn_val;
	u32 tu32Temp = 0;
	u16 tu16Temp = 0;
	u32 Res1, Res2, Res3;

	sys_init();			// 系统初始化
	uart2_init(115200); // 初始化串口2
	Temp_Init();
	Hat_Time = 0;

	val = 3;
	Write_Flash = -1;
	Hat_State = 0;
	write_page(0);
	norflash_read(0x000000, (u8 *)&tu32Temp, 2);
	Set_Hat_Time = tu32Temp;
	if (Set_Hat_Time < 0)
	{
		Set_Hat_Time = 0;
	}
	else if (Set_Hat_Time > 600)
	{
		Set_Hat_Time = 0;
	}
	norflash_read(0x000004, (u8 *)&tu32Temp, 2);
	Set_Hat_Temp = tu32Temp;
	if (Set_Hat_Temp < 0)
	{
		Set_Hat_Temp = 0;
	}
	else if (Set_Hat_Temp > 99)
	{
		Set_Hat_Temp = 99;
	}
	norflash_read(0x000008, (u8 *)&tu32Temp, 2);
	Set_War_Temp = tu32Temp;
	if (Set_War_Temp < 0)
	{
		Set_War_Temp = 0;
	}
	else if (Set_War_Temp > 1)
	{
		Set_War_Temp = 0;
	}

	norflash_read(0x00000C, (u8 *)&tu32Temp, 2);
	Set_Temp_xs = tu32Temp;
	if (Set_Temp_xs < 0)
	{
		Set_Temp_xs = 0;
	}
	else if (Set_Temp_xs > 1000)
	{
		Set_Temp_xs = 1000;
	}

	norflash_read(0x000010, (u8 *)&tu32Temp, 2);
	Set_Temp_xs2 = tu32Temp;
	if (Set_Temp_xs2 < 0)
	{
		Set_Temp_xs2 = 0;
	}
	else if (Set_Temp_xs2 > 1000)
	{
		Set_Temp_xs2 = 1000;
	}

	sys_write_vp(0x2000, (u8 *)&Set_Hat_Temp, 1);
	sys_write_vp(0x2002, (u8 *)&Set_Hat_Time, 1);
	sys_write_vp(0x2004, (u8 *)&Set_Hat_Time, 1);
	sys_write_vp(0x2005, (u8 *)&Set_Hat_Temp, 1);
	sys_write_vp(0x2006, (u8 *)&Set_War_Temp, 1);
	sys_write_vp(0x2008, (u8 *)&Set_Temp_xs, 1);
	sys_write_vp(0x2009, (u8 *)&Set_Temp_xs2, 1);

	//	tu32Temp = 1;
	//	sys_write_vp(0x8000,(u8*)&tu32Temp,2);

	//	tu32Temp = Set_Hat_Time;
	//	sys_write_vp(0x8002,(u8*)&tu32Temp,2);

	//	sys_read_vp(0x2006,(u8*)&Set_War_Temp,1);

	while (1)
	{
		//		if(Task_Tick % 500 == 0)
		{
			//			USART_Send();
		}
		if (((Set_Hat_Temp != Set_Hat_Temp_old) ||
			 (Set_Hat_Time != Set_Hat_Time_old) ||
			 (Set_War_Temp != Set_War_Temp_old)) ||
			(Set_Temp_xs_old != Set_Temp_xs) ||
			(Set_Temp_xs_old2 != Set_Temp_xs2))
		{
			Write_Flash = 5000;
			//			tu32Temp = 6;
			//			sys_write_vp(0x8000,(u8*)&tu32Temp,2);
			Set_Hat_Temp_old = Set_Hat_Temp;
			Set_Hat_Time_old = Set_Hat_Time;
			Set_War_Temp_old = Set_War_Temp;
			Set_Temp_xs_old = Set_Temp_xs;
			Set_Temp_xs_old2 = Set_Temp_xs2;
			sys_write_vp(0x2000, (u8 *)&Set_Hat_Temp, 1);
			sys_write_vp(0x2002, (u8 *)&Set_Hat_Time, 1);
		}
		//		sys_write_vp(0x8002,(u8*)&Write_Flash,2);
		if (Write_Flash == 0)
		{
			tu32Temp = Set_Hat_Time;
			norflash_write(0x000000, (u8 *)&tu32Temp, 2);
			tu32Temp = Set_Hat_Temp;
			norflash_write(0x000004, (u8 *)&tu32Temp, 2);
			tu32Temp = Set_War_Temp;
			norflash_write(0x000008, (u8 *)&tu32Temp, 2);
			tu32Temp = Set_Temp_xs;
			norflash_write(0x00000C, (u8 *)&tu32Temp, 2);
			tu32Temp = Set_Temp_xs2;
			norflash_write(0x000010, (u8 *)&tu32Temp, 2);
			Write_Flash = -1;
			tu32Temp = 7;
			sys_write_vp(0x8000, (u8 *)&tu32Temp, 2);
		}
		if (Task_Tick % 50 == 0)
		{
			if (Hat_State == 1)
			{
				tu16Temp = (u16)(((float)Hat_Time / 6000.0f) + 0.5);
				sys_write_vp(0x2003, (u8 *)&tu16Temp, 1);

				tu16Temp = 100 - ((Hat_Time / 60) / Set_Hat_Time);
				sys_write_vp(0x2100, (u8 *)&tu16Temp, 1);
				sys_write_vp(0x2102, (u8 *)&tu16Temp, 1);
			}
			else
			{
				tu16Temp = 0;
				sys_write_vp(0x2003, (u8 *)&tu16Temp, 1);

				tu16Temp = 0;
				sys_write_vp(0x2100, (u8 *)&tu16Temp, 1);
				sys_write_vp(0x2102, (u8 *)&tu16Temp, 1);
			}
		}
		if (uart2_rx_sta & UART2_PACKET_OK) // 接受到了串口数据包
		{
			len = uart2_rx_sta & UART2_PACKET_LEN; // 得到串口数据包的长度,不包含"\r\n"或者'\n'结束符的长度
			//			uart2_buf[len++] = 0;//在末尾添加2个空字符
			//			uart2_buf[len++] = 0;

			//			printf("T5L_C51:%s\r\n",uart2_buf);//把接受到的数据包加上"T5L_C51:"前缀后返还给发送者
			//			sys_write_vp(0x2000,uart2_buf,len/2+1);//同时把数据包显示到界面上

			if (len == 14 && uart2_buf[0] == 0xFA && uart2_buf[13] == 0xAF)
			{
				Res1 = (u32)((u32)uart2_buf[1] << 24) + (u32)((u32)uart2_buf[2] << 16) + (u32)((u32)uart2_buf[3] << 8) + uart2_buf[4];
				Res2 = (u32)((u32)uart2_buf[5] << 24) + (u32)((u32)uart2_buf[6] << 16) + (u32)((u32)uart2_buf[7] << 8) + uart2_buf[8];
				Res3 = (u32)((u32)uart2_buf[9] << 24) + (u32)((u32)uart2_buf[10] << 16) + (u32)((u32)uart2_buf[11] << 8) + uart2_buf[12];
				Temp1 = GET_Temp(Res1);
				Temp2 = GET_Temp(Res2);
				Temp3 = GET_Temp(Res3);
				//				sys_write_vp(0x2000,(u8*)&Temp1,1);
				sys_write_vp(0x2001, (u8 *)&Temp1, 1);
				sys_write_vp(0x2010, (u8 *)&Temp2, 1);

				//				sys_write_vp(0x8000,(u8*)&Res1,2);
				//				sys_write_vp(0x8002,(u8*)&Res2,2);
				SW1_State = uart2_buf[9];
				SW2_State = uart2_buf[10];

				USART_Send();
			}

			uart2_rx_sta = 0; // 清0代表处理掉了此串口包
		}
		sys_read_vp(0x2004, (u8 *)&Set_Hat_Time, 1);
		sys_read_vp(0x2005, (u8 *)&Set_Hat_Temp, 1);
		sys_read_vp(0x2006, (u8 *)&Set_War_Temp, 1);
		sys_write_vp(0x3000, (u8 *)&Set_Hat_Time, 1);
		sys_read_vp(0x2008, (u8 *)&Set_Temp_xs, 1);
		sys_read_vp(0x2009, (u8 *)&Set_Temp_xs2, 1);
		if ((Temp1 > 128 || Temp2 > 128 || Temp3 > 128) && (Task_Tick % 10000 == 0))
		{
			write_page(2);
		}

		if (Hat_Time == 0 && Hat_State == 1)
		{
			write_beep(0x3E);
			sys_delay_about_ms(1000);
			write_beep(0x3E);
			sys_delay_about_ms(1000);
			write_beep(0x3E);
			Hat_Time = 0;
			write_page(0);
			Hat_State = 0;
		}

		sys_read_vp(0x1000, (u8 *)&btn_val, 1);
		switch (btn_val)
		{
		case 0x0001:
		{
			//				if(SW1_State | SW2_State)
			//				{
			write_page(1);
			Hat_Time = (u32)Set_Hat_Time * 6000;
			Hat_State = 1;
			//				}
			//				else
			//				{
			//					write_page(8);
			//				}
		}
		break;
		case 0x0002:
		{
			Hat_Time = 0;
			write_page(0);
			Hat_State = 0;
		}
		break;
		case 0x0003:
		{
			write_page(3);
		}
		break;
		case 0x0004:
		{
			write_page(4);
		}
		break;
		case 0x0010:
		case 0x0011:
		case 0x0012:
		case 0x0013:
		{
			write_page(0);
		}
		break;
		case 0x0014:
		case 0x0015:
		{
			if (Hat_Time > 50)
			{
				write_page(1);
			}
			else
			{
				write_page(0);
			}
		}
		break;
		}
		if (btn_val != 0)
		{
			btn_val = 0;
			sys_write_vp(0x1000, (u8 *)&btn_val, 1);
		}
	}
}
