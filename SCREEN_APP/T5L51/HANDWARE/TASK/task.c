#include "task.h"
#include "uart2.h"
#include "nor_flash.h"


xdata u16 Temp1,Temp2,Temp3;
xdata u8 Tx_Buf[20];
xdata u8 Hat_State;
xdata u8 SW1_State, SW2_State;
xdata u32 Task_Tick;
xdata u32 Hat_Time;
xdata u16 Set_Hat_Time;
xdata u16 Set_Hat_Temp;
xdata u16 Set_War_Temp;
xdata u16 Set_Temp_xs;
xdata u16 Set_Temp_xs2;
xdata s32  Write_Flash;
xdata u16 Set_Hat_Time_old;
xdata u16 Set_Hat_Temp_old;
xdata u16 Set_War_Temp_old;
xdata u16 Set_Temp_xs_old;
xdata u16 Set_Temp_xs_old2;



//50k
xdata u32 Resistance_table[150];
void Temp_Init(void)
{
	Resistance_table[0] =     163300,   //0         40
	Resistance_table[1] =     155200;   //1         41
	Resistance_table[2] =     147500;   //2         42
	Resistance_table[3] =     140300;   //3         43
 	Resistance_table[4] =     133400;   //4         44
	Resistance_table[5] =     127000;   //5         45
	Resistance_table[6] =     120900;   //6         46
	Resistance_table[7] =     115100;   //7         47
	Resistance_table[8] =     109600;   //8         48
	Resistance_table[9] =     104400;   //9         49
	Resistance_table[10] =     99500;   //10        50
	Resistance_table[11] =     94850;   //11        51
	Resistance_table[12] =     90450;   //12        52
	Resistance_table[13] =     86300;   //13        53
	Resistance_table[14] =     82300;   //14        54
	Resistance_table[15] =     78550;   //15        55
	Resistance_table[16] =     75000;   //16        56
	Resistance_table[17] =     71600;   //17        57
	Resistance_table[18] =     68400;   //18        58
	Resistance_table[19] =     65350;   //19        59
	Resistance_table[20] =     62450;   //20        60
	Resistance_table[21] =     59700;   //21        61
	Resistance_table[22] =     57100;   //22        62
	Resistance_table[23] =     54600;   //23        63
	Resistance_table[24] =     52250;   //24        64
	Resistance_table[25] =     50000;   //25        65
 	Resistance_table[26] =    47870;   //26        66
 	Resistance_table[27] =    45830;   //27        67
 	Resistance_table[28] =    43890;   //28        68
 	Resistance_table[29] =    42040;   //29        69
 	Resistance_table[30] =    40290;   //30        70
 	Resistance_table[31] =    38610;   //31        71
 	Resistance_table[32] =    37020;   //32        72
 	Resistance_table[33] =    35490;   //33        73
 	Resistance_table[34] =    34040;   //34        74
 	Resistance_table[35] =    32660;   //35        75
 	Resistance_table[36] =    31340;   //36        76
 	Resistance_table[37] =    30080;   //37        77
 	Resistance_table[38] =        28880;   //38        78
 	Resistance_table[39] =        27730;   //39        79
 	Resistance_table[40] =        26630;   //40        80
 	Resistance_table[41] =        25590;   //41        81
 	Resistance_table[42] =        24590;   //42        82
 	Resistance_table[43] =        23630;   //43        83
 	Resistance_table[44] =        22720;   //44        84
 	Resistance_table[45] =        21840;   //45        85
 	Resistance_table[46] =        21010;   //46        86
 	Resistance_table[47] =        20210;   //47        87
 	Resistance_table[48] =        19440;   //48        88
 	Resistance_table[49] =        18710;   //49        89
 	Resistance_table[50] =        18010;   //50        90
 	Resistance_table[51] =        17340;   //51        91
 	Resistance_table[52] =        16700;   //52        92
 	Resistance_table[53] =        16080 ;   //53        93
 	Resistance_table[54] =        15490 ;   //54        94
 	Resistance_table[55] =        14930 ;   //55        95
 	Resistance_table[56] =        14390 ;   //56        96
 	Resistance_table[57] =        13870 ;   //57        97
 	Resistance_table[58] =        13370 ;   //58        98
 	Resistance_table[59] =        12900 ;   //59        99
 	Resistance_table[60] =        12400 ;   //60        100
 	Resistance_table[61] =        12000 ;   //61        101
 	Resistance_table[62] =        11580 ;   //62        102
 	Resistance_table[63] =        11170 ;   //63        103
 	Resistance_table[64] =        10790 ;   //64        104
 	Resistance_table[65] =        10410 ;   //65        105
 	Resistance_table[66] =        10060 ;   //66        106
 	Resistance_table[67] =        9710 ;   //67        107
 	Resistance_table[68] =        9380 ;   //68        108
 	Resistance_table[69] =        9065 ;   //69        109
 	Resistance_table[70] =        8755 ;   //70        110
 	Resistance_table[71] =        8465 ;   //71        111
 	Resistance_table[72] =        8185 ;   //72        112
 	Resistance_table[73] =        7910 ;   //73        113
 	Resistance_table[74] =        7650 ;   //74        114
 	Resistance_table[75] =        7400 ;   //75        115
 	Resistance_table[76] =        7160 ;   //76        116
 	Resistance_table[77] =        6925 ;   //77        117
 	Resistance_table[78] =        6705 ;   //78        118
 	Resistance_table[79] =        6490 ;   //79        119
 	Resistance_table[80] =        6280 ;   //80        120
 	Resistance_table[81] =        6080 ;   //81        121
 	Resistance_table[82] =        5890 ;   //82        122
 	Resistance_table[83] =        5705 ;   //83        123
 	Resistance_table[84] =        5525 ;   //84        124
 	Resistance_table[85] =        5355 ;   //85        125
 	Resistance_table[86] =        5190 ;   //86        126
 	Resistance_table[87] =        5030 ;   //87        127
 	Resistance_table[88] =        4875 ;   //88        128
 	Resistance_table[89] =        4726 ;   //89        129
 	Resistance_table[90] =        4582 ;   //90        130
 	Resistance_table[91] =        4444 ;   //91        131
 	Resistance_table[92] =        4310 ;   //92        132
 	Resistance_table[93] =        4182 ;   //93        133
 	Resistance_table[94] =        4057 ;   //94        134
 	Resistance_table[95] =        3937 ;   //95        135
 	Resistance_table[96] =        3821 ;   //96        136
 	Resistance_table[97] =        3709 ;   //97        137
 	Resistance_table[98] =        3601 ;   //98        138
 	Resistance_table[99] =        3497 ;   //99        139
 	Resistance_table[100] =        3396 ;   //99        139
 	Resistance_table[101] =        3298 ;   //99        139
 	Resistance_table[102] =        3204 ;   //99        139
 	Resistance_table[103] =        3113 ;   //99        139
 	Resistance_table[104] =        3025 ;   //99        139
 	Resistance_table[105] =        2940 ;   //99        139
 	Resistance_table[106] =        2857 ;   //99        139
 	Resistance_table[107] =        2778 ;   //99        139
 	Resistance_table[108] =        2701 ;   //99        139
 	Resistance_table[109] =        2626 ;   //99        139
 	Resistance_table[110] =        2554 ;   //99        139
 	Resistance_table[111] =        2484 ;   //99        139
 	Resistance_table[112] =        2416 ;   //99        139
 	Resistance_table[113] =        2351 ;   //99        139
 	Resistance_table[114] =        2287 ;   //99        139
 	Resistance_table[115] =        2226 ;   //99        139
 	Resistance_table[116] =        2167 ;   //99        139
 	Resistance_table[117] =        2109 ;   //99        139
 	Resistance_table[118] =        2053 ;   //99        139
 	Resistance_table[119] =        1999 ;   //99        139
 	Resistance_table[120] =        1947 ;   //99        139
 	Resistance_table[121] =        1896 ;   //99        139
 	Resistance_table[122] =        1847 ;   //99        139
 	Resistance_table[123] =        1799 ;   //99        139
 	Resistance_table[124] =        1610 ;   //99        139
 	Resistance_table[125] =        1566 ;   //99        139
 	Resistance_table[126] =        1524 ;   //99        139
 	Resistance_table[127] =        1483 ;   //99        139
 	Resistance_table[128] =        1443 ;   //99        139
 	Resistance_table[129] =        1404 ;   //99        139
 	Resistance_table[130] =        1367 ;   //99        139
}

u16 GET_Temp(u32 Res)
{
	u16 Temp_Value = 0;
	u8 i = 0;
			for(; i < 130; i++)                 //数据的三种情况，等于，小于和大于
			{
				Temp_Value = i; 
					if(Res >= Resistance_table[i])
					{
//							Temp_Value =  i;               //换算成温度减40
							break;
					}
		}
//		if(Temp_Value <= 40)	
//		{
//			Temp_Value *= 1.02; 
//		}
//		else if(Temp_Value <= 60)
//		{
//			Temp_Value *= 1.05; 
//		}
//		else if(Temp_Value > 60)
		{
			Temp_Value *= Set_Temp_xs * 0.01; 
		}
		return Temp_Value;
}


void USART_Send(void)
{
	Tx_Buf[0] = 0xFA;
	Tx_Buf[1] = Temp1;
	Tx_Buf[2] = Temp2;
	Tx_Buf[3] = Temp3;
	Tx_Buf[4] = Hat_State;
	Tx_Buf[5] = Set_Hat_Temp;
	Tx_Buf[6] = Set_War_Temp;
	Tx_Buf[7] = 0xAF;
	u2_send_bytes(Tx_Buf, 8);
}

