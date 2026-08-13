#include "usart_drive.h"

ST_USART st_Uart1 = {{0}, {0}, 0, 0};

/* Timestamp (HAL tick) of the last valid packet from the screen.
   Updated in the UART RX-event callback; used for the comm-timeout safety check. */
volatile uint32_t g_last_rx_tick = 0;
