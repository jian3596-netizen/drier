#include "dwin_drive.h"
#include "control.h"
#include "usart.h"          /* huart1 */

/*
 * Frame sent to the DWIN screen:
 *   FA | res0(4) | res1(4) | res2(4) | preheat_stop | AF | '\n'
 * Bytes 1..12 are filled by GET_ADC(). preheat_stop is a reliable request:
 * after the one-hour timeout it remains 1 until the screen replies with its
 * preheat flag cleared. The screen then writes 0 to the existing VP 0x2006.
 */
void DWIN_Send(void)
{
    st_Uart1.a_Tx_Buf[0]  = 0xFA;
    st_Uart1.a_Tx_Buf[13] = Control_PreheatStopPending();
    st_Uart1.a_Tx_Buf[14] = 0xAF;
    st_Uart1.a_Tx_Buf[15] = '\n';
    HAL_UART_Transmit(&huart1, st_Uart1.a_Tx_Buf, 16, 1000);
}
