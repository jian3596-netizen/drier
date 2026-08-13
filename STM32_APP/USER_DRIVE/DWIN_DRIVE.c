#include "dwin_drive.h"
#include "control.h"
#include "usart.h"          /* huart1 */

/*
 * Normal frame (kept byte-for-byte compatible with the original screen):
 *   FA | res0(4) | res1(4) | res2(4) | AF | '\n'
 *
 * Forced preheat-stop command:
 *   FB | 57 | 01 | BF | '\n'
 *
 * While a stop acknowledgement is pending, alternate normal and command
 * frames. The T5L51 receiver accepts only one complete frame at a time, so
 * back-to-back frames without a main-loop interval could lose one.
 */
void DWIN_Send(void)
{
    static uint8_t send_stop_command = 0;

    if (Control_PreheatStopPending() && send_stop_command)
    {
        st_Uart1.a_Tx_Buf[0] = 0xFB;
        st_Uart1.a_Tx_Buf[1] = 0x57;
        st_Uart1.a_Tx_Buf[2] = 0x01;
        st_Uart1.a_Tx_Buf[3] = 0xBF;
        st_Uart1.a_Tx_Buf[4] = '\n';
        HAL_UART_Transmit(&huart1, st_Uart1.a_Tx_Buf, 5, 1000);
        send_stop_command = 0;
        return;
    }

    st_Uart1.a_Tx_Buf[0]  = 0xFA;
    st_Uart1.a_Tx_Buf[13] = 0xAF;
    st_Uart1.a_Tx_Buf[14] = '\n';
    HAL_UART_Transmit(&huart1, st_Uart1.a_Tx_Buf, 15, 1000);
    send_stop_command = Control_PreheatStopPending() ? 1 : 0;
}
