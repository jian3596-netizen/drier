#include "nor_flash.h"



xdata u8 norflash_cmd[8];


//向T5L内部自带的nor flash写入数据
//addr:nor flash的地址,必须为偶数,范围为:0x000000-0x027FFE,然后一个地址对应2个字节,即总容量为320KB
//buff:需要写入的缓冲区,长度至少大于len*2
//len:写入的字数,必须为偶数,而且写入的最大长度也有限制,这个跟CACHE_ADDR的值有关系
void norflash_write(u32 addr,u8* buff,u16 len)
{
	//1.先把buff中的数据写入到DGUSII可用变量区(CACHE_ADDR)缓存一下
	sys_write_vp(CACHE_ADDR,buff,len);
	
	//2.然后再把CACHE_ADDR处的内容写入到nor flash中
	norflash_cmd[0] = NOR_FLASH_WRITE;//写操作
	norflash_cmd[1] = (addr>>16)&0xff;//nor flash地址
	norflash_cmd[2] = (addr>>8)&0xff;
	norflash_cmd[3] = addr&0xff;
	norflash_cmd[4] = (CACHE_ADDR>>8)&0xff;//dgusii数据地址
	norflash_cmd[5] = CACHE_ADDR&0xff;
	norflash_cmd[6] = (len>>8)&0xff;//写入的数据长度
	norflash_cmd[7] = len&0xff;
	sys_write_vp(NOR_FLASH_ADDR,norflash_cmd,4);
	
	//3.等待操作完成
	while(1)
	{
		sys_read_vp(NOR_FLASH_ADDR,norflash_cmd,2);//只需读取0x0008变量的前2个字,然后判断D7是否为0
		if(norflash_cmd[0]==0)
			break;
		sys_delay_about_ms(1);//这个延时必须加,可以防止莫名其妙的错误
	}
}



//向T5L内部自带的nor flash读取数据
//addr:nor flash的地址,必须为偶数,范围为:0x000000-0x027FFE,然后一个地址对应2个字节,即总容量为320KB
//buff:需要读入的缓冲区,长度至少大于len*2
//len:读取的字数,必须为偶数,而且写入的最大长度也有限制,这个跟CACHE_ADDR的值有关系
void norflash_read(u32 addr,u8* buff,u16 len)
{
	//1.把nor flash中的内容读取到CACHE_ADDR
	norflash_cmd[0] = NOR_FLASH_READ;//读操作
	norflash_cmd[1] = (addr>>16)&0xff;//nor flash地址
	norflash_cmd[2] = (addr>>8)&0xff;
	norflash_cmd[3] = addr&0xff;
	norflash_cmd[4] = (CACHE_ADDR>>8)&0xff;//dgusii数据地址
	norflash_cmd[5] = CACHE_ADDR&0xff;
	norflash_cmd[6] = (len>>8)&0xff;//写入的数据长度
	norflash_cmd[7] = len&0xff;
	sys_write_vp(NOR_FLASH_ADDR,norflash_cmd,4);
	
	//2.等待操作完成
	while(1)
	{
		sys_read_vp(NOR_FLASH_ADDR,norflash_cmd,2);//只需读取0x0008变量的前4个字节,然后判断D7是否为0
		if(norflash_cmd[0]==0)
			break;
		sys_delay_about_ms(1);//这个延时必须加,可以防止莫名其妙的错误
	}
	
	//3.再把CACHE_ADDR处的数据读取到buff中
	sys_read_vp(CACHE_ADDR,buff,len);
}



















