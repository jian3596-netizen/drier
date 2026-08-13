#include "adc_drive.h"

uint16_t AD_Buf[AD_Buf_LEN];
uint32_t g_Res[3];          /* NTC resistances (ohm), channels 0..2 */
uint8_t  Voltage;           /* supply voltage sense (for future under-volt check) */

/*
 * Average the DMA buffer (4 interleaved channels, 8 samples each), then:
 *   - ch3 -> supply voltage
 *   - ch0..2 -> NTC resistances (6.8k divider) into g_Res[] and the TX buffer.
 * All integer math (no float) so we don't pull in the soft-float library.
 *
 * Node voltage:  v_mV = acc * 3300 / 4096 + offset
 * NTC resistance (6.8k divider, NTC on the 3.3V side):
 *   R = 6800 * (3300 - v_mV) / v_mV     [ohm]
 *   open  -> v_mV small -> R huge ; short -> v_mV>=3300 -> R set 0  (both = fault)
 * Resistances are sent to the screen, which converts them to temperature.
 */
void GET_ADC(void)
{
    uint32_t acc[4] = {0, 0, 0, 0};
    uint8_t  i;
    uint32_t v_mV;

    for (i = 0; i < AD_Buf_LEN; )
    {
        acc[0] += AD_Buf[i++];
        acc[1] += AD_Buf[i++];
        acc[2] += AD_Buf[i++];
        acc[3] += AD_Buf[i++];
    }
    acc[0] >>= 3;
    acc[1] >>= 3;
    acc[2] >>= 3;
    acc[3] >>= 3;

    /* Supply voltage sense (ch3). */
    v_mV = (acc[3] * 3300U) / 4096U + 350U;
    Voltage = (uint8_t)(v_mV * 111U / 1000U);

    /* NTC resistances (ch0..2). */
    for (i = 0; i < 3; i++)
    {
        v_mV = (acc[i] * 3300U) / 4096U + 60U;
        if (v_mV >= 3300U)
            g_Res[i] = 0;                                 /* short / over-range -> fault */
        else
            g_Res[i] = 6800U * (3300U - v_mV) / v_mV;     /* open -> very large -> fault */
    }

    /* Pack the 3 resistances (big-endian, 4 bytes each) into the TX buffer.
       DWIN_Send() adds the 0xFA header / 0xAF tail and transmits. */
    st_Uart1.a_Tx_Buf[1]  = (uint8_t)(g_Res[0] >> 24);
    st_Uart1.a_Tx_Buf[2]  = (uint8_t)(g_Res[0] >> 16);
    st_Uart1.a_Tx_Buf[3]  = (uint8_t)(g_Res[0] >> 8);
    st_Uart1.a_Tx_Buf[4]  = (uint8_t)(g_Res[0]);
    st_Uart1.a_Tx_Buf[5]  = (uint8_t)(g_Res[1] >> 24);
    st_Uart1.a_Tx_Buf[6]  = (uint8_t)(g_Res[1] >> 16);
    st_Uart1.a_Tx_Buf[7]  = (uint8_t)(g_Res[1] >> 8);
    st_Uart1.a_Tx_Buf[8]  = (uint8_t)(g_Res[1]);
    st_Uart1.a_Tx_Buf[9]  = (uint8_t)(g_Res[2] >> 24);
    st_Uart1.a_Tx_Buf[10] = (uint8_t)(g_Res[2] >> 16);
    st_Uart1.a_Tx_Buf[11] = (uint8_t)(g_Res[2] >> 8);
    st_Uart1.a_Tx_Buf[12] = (uint8_t)(g_Res[2]);
}
