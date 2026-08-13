#include "sys.h"
#include "uart2.h"
#include "task.h"
#include "nor_flash.h"

T5L_XDATA u16 val;

#define SETTINGS_VERSION_ADDR 0x000014UL
#define SETTINGS_VERSION      0x44525901UL

#define DEFAULT_HAT_TIME_MIN  10U
#define DEFAULT_HAT_TEMP_C    60U
#define DEFAULT_WARM_ENABLE   0U
#define DEFAULT_TEMP_SCALE    100U

static u8 sanitize_settings(void)
{
	u8 changed = 0;

	if (Set_Hat_Time > 600)
	{
		Set_Hat_Time = 0;
		changed = 1;
	}
	if (Set_Hat_Temp > 99)
	{
		Set_Hat_Temp = 0;
		changed = 1;
	}
	if (Set_War_Temp > 1)
	{
		Set_War_Temp = 0;
		changed = 1;
	}
	if (Set_Temp_xs < 50 || Set_Temp_xs > 200)
	{
		Set_Temp_xs = 100;
		changed = 1;
	}
	if (Set_Temp_xs2 < 50 || Set_Temp_xs2 > 200)
	{
		Set_Temp_xs2 = 100;
		changed = 1;
	}

	return changed;
}

static void set_default_settings(void)
{
	Set_Hat_Time = DEFAULT_HAT_TIME_MIN;
	Set_Hat_Temp = DEFAULT_HAT_TEMP_C;
	Set_War_Temp = DEFAULT_WARM_ENABLE;
	Set_Temp_xs = DEFAULT_TEMP_SCALE;
	Set_Temp_xs2 = DEFAULT_TEMP_SCALE;
}

static void sync_settings_snapshot(void)
{
	Set_Hat_Time_old = Set_Hat_Time;
	Set_Hat_Temp_old = Set_Hat_Temp;
	Set_War_Temp_old = Set_War_Temp;
	Set_Temp_xs_old = Set_Temp_xs;
	Set_Temp_xs_old2 = Set_Temp_xs2;
}

static void save_settings(void)
{
	norflash_write_u32(0x000000, Set_Hat_Time);
	norflash_write_u32(0x000004, Set_Hat_Temp);
	/* Preheat is a runtime state: never persist ON across a power cycle. */
	norflash_write_u32(0x000008, DEFAULT_WARM_ENABLE);
	norflash_write_u32(0x00000C, Set_Temp_xs);
	norflash_write_u32(0x000010, Set_Temp_xs2);
	/* Write the version last so an interrupted first save is retried at next boot. */
	norflash_write_u32(SETTINGS_VERSION_ADDR, SETTINGS_VERSION);
}

void main(void)
{
	u16 len;
	u16 btn_val;
	u32 tu32Temp = 0;
	u16 tu16Temp = 0;
	u8 settings_changed = 0;
	u32 Res1, Res2, Res3;
	u32 ui_tick = 0;
	u32 alarm_tick = 0;
	u32 now_tick;

	sys_init();			// 系统初始化
	uart2_init(115200); // 初始化串口2
	Temp_Init();
	Hat_Time = 0;

	val = 3;
	Write_Flash = -1;
	Hat_State = 0;
	write_page(0);
	norflash_read_u32(SETTINGS_VERSION_ADDR, &tu32Temp);
	if (tu32Temp == SETTINGS_VERSION)
	{
		norflash_read_u32(0x000000, &tu32Temp);
		Set_Hat_Time = tu32Temp;
		norflash_read_u32(0x000004, &tu32Temp);
		Set_Hat_Temp = tu32Temp;
		norflash_read_u32(0x000008, &tu32Temp);
		Set_War_Temp = tu32Temp;
		norflash_read_u32(0x00000C, &tu32Temp);
		Set_Temp_xs = tu32Temp;
		norflash_read_u32(0x000010, &tu32Temp);
		Set_Temp_xs2 = tu32Temp;
		settings_changed = sanitize_settings();
		if (Set_War_Temp != DEFAULT_WARM_ENABLE)
		{
			Set_War_Temp = DEFAULT_WARM_ENABLE;
			settings_changed = 1;
		}
		if (settings_changed)
			save_settings();
	}
	else
	{
		set_default_settings();
		save_settings();
	}
	sync_settings_snapshot();

	sys_write_vp_u16(0x2000, Set_Hat_Temp);
	sys_write_vp_u16(0x2002, Set_Hat_Time);
	sys_write_vp_u16(0x2004, Set_Hat_Time);
	sys_write_vp_u16(0x2005, Set_Hat_Temp);
	sys_write_vp_u16(0x2006, Set_War_Temp);
	sys_write_vp_u16(0x2008, Set_Temp_xs);
	sys_write_vp_u16(0x2009, Set_Temp_xs2);

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
			 (Set_Hat_Time != Set_Hat_Time_old)) ||
			(Set_Temp_xs_old != Set_Temp_xs) ||
			(Set_Temp_xs_old2 != Set_Temp_xs2))
		{
			Write_Flash = 5000;
			//			tu32Temp = 6;
			//			sys_write_vp(0x8000,(u8*)&tu32Temp,2);
			Set_Hat_Temp_old = Set_Hat_Temp;
			Set_Hat_Time_old = Set_Hat_Time;
			Set_Temp_xs_old = Set_Temp_xs;
			Set_Temp_xs_old2 = Set_Temp_xs2;
			sys_write_vp_u16(0x2000, Set_Hat_Temp);
			sys_write_vp_u16(0x2002, Set_Hat_Time);
		}
		//		sys_write_vp(0x8002,(u8*)&Write_Flash,2);
		if (Write_Flash == 0)
		{
			save_settings();
			Write_Flash = -1;
			tu32Temp = 7;
			sys_write_vp_u32(0x8000, tu32Temp);
		}
		now_tick = Task_Tick;
		if ((u32)(now_tick - ui_tick) >= 50UL)
		{
			ui_tick = now_tick;
			if (Hat_State == 1)
			{
				tu16Temp = (u16)((Hat_Time + 3000UL) / 6000UL);
				sys_write_vp_u16(0x2003, tu16Temp);

				if (Set_Hat_Time != 0)
					tu16Temp = 100 - ((Hat_Time / 60) / Set_Hat_Time);
				else
					tu16Temp = 0;
				sys_write_vp_u16(0x2100, tu16Temp);
				sys_write_vp_u16(0x2102, tu16Temp);
			}
			else
			{
				tu16Temp = 0;
				sys_write_vp_u16(0x2003, tu16Temp);

				tu16Temp = 0;
				sys_write_vp_u16(0x2100, tu16Temp);
				sys_write_vp_u16(0x2102, tu16Temp);
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
				Temp2 = GET_Temp_Calibrated(Res2, Set_Temp_xs2);
				Temp3 = GET_Temp(Res3);
				//				sys_write_vp(0x2000,(u8*)&Temp1,1);
				sys_write_vp_u16(0x2001, Temp1);
				sys_write_vp_u16(0x2010, Temp2);

				//				sys_write_vp(0x8000,(u8*)&Res1,2);
				//				sys_write_vp(0x8002,(u8*)&Res2,2);
				SW1_State = uart2_buf[9];
				SW2_State = uart2_buf[10];

				USART_Send();
			}
			else if (len == 4 && uart2_buf[0] == 0xFB &&
					 uart2_buf[1] == 0x57 && uart2_buf[3] == 0xBF)
			{
				if (uart2_buf[2] != 0)
				{
					Set_War_Temp = 0;
					sys_write_vp_u16(0x2006, Set_War_Temp);
				}
				USART_Send();
			}

			uart2_release_packet();
		}
		sys_read_vp_u16(0x2004, &Set_Hat_Time);
		sys_read_vp_u16(0x2005, &Set_Hat_Temp);
		sys_read_vp_u16(0x2006, &Set_War_Temp);
		sys_write_vp_u16(0x3000, Set_Hat_Time);
		sys_read_vp_u16(0x2008, &Set_Temp_xs);
		sys_read_vp_u16(0x2009, &Set_Temp_xs2);
		sanitize_settings();
		if ((Temp1 > 128 || Temp2 > 128 || Temp3 > 128) &&
			(u32)(now_tick - alarm_tick) >= 10000UL)
		{
			alarm_tick = now_tick;
			write_page(2);
		}

		if (Hat_Time == 0 && Hat_State == 1)
		{
			write_beep(0x3E);
			sys_delay_ms(1000);
			write_beep(0x3E);
			sys_delay_ms(1000);
			write_beep(0x3E);
			Hat_Time = 0;
			write_page(0);
			Hat_State = 0;
		}

		sys_read_vp_u16(0x1000, &btn_val);
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
			sys_write_vp_u16(0x1000, btn_val);
		}
	}
}
