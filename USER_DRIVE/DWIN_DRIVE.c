#include "dwin_drive.h"
#include "usart.h"          /* huart1 */

/*
 * Frame sent to the DWIN screen:
 *   FA | res0(4) | res1(4) | res2(4) | AF | '\n'
 * Bytes 1..12 are filled by GET_ADC(); here we add header/tail and transmit.
 * The screen converts the resistances to temperature and displays them.
 */
void DWIN_Send(void)
{
    st_Uart1.a_Tx_Buf[0]  = 0xFA;
    st_Uart1.a_Tx_Buf[13] = 0xAF;
    st_Uart1.a_Tx_Buf[14] = '\n';
    HAL_UART_Transmit(&huart1, st_Uart1.a_Tx_Buf, 15, 1000);
}
