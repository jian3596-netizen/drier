#ifndef __USART_DRIVE_H
#define __USART_DRIVE_H

#include "main.h"

typedef struct
{
    uint8_t  a_Rx_Buf[20];      /* receive buffer                                   */
    uint8_t  a_Tx_Buf[20];      /* transmit buffer                                  */
    uint8_t  USART_RX_SUCCESS;  /* set when a complete valid screen packet arrives  */
    uint8_t  Rx_Len;            /* length of the last received packet               */
} ST_USART;

extern ST_USART st_Uart1;
extern volatile uint32_t g_last_rx_tick;   /* HAL_GetTick() at last valid screen packet */

#endif /* __USART_DRIVE_H */
