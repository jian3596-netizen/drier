#ifndef __NOR_FLASH_H__
#define __NOR_FLASH_H_
#include "sys.h"

//宏定义
#define NOR_FLASH_ADDR			0x0008	//系统变量地址
#define NOR_FLASH_WRITE			0xA5		//进行写操作
#define NOR_FLASH_READ			0x5A		//进行读操作

#define CACHE_ADDR					0xFF00	//0XFF00-0XFFFF作为缓冲区域,大概有256个字,512个字节,注意,GUI开发时不要使用此区域




//函数申明
void norflash_write(u32 addr,u8* buff,u16 len);
void norflash_read(u32 addr,u8* buff,u16 len);
#endif

