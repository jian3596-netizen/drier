#ifndef __ADC_DRIVE_H
#define __ADC_DRIVE_H

#include "main.h"

#define AD_Buf_LEN  32          /* 4 channels x 8 samples */

extern uint16_t AD_Buf[AD_Buf_LEN];   /* raw ADC DMA buffer                          */
extern uint32_t g_Res[3];             /* NTC resistances in ohm (ch0..2), used for safety */

void GET_ADC(void);

#endif /* __ADC_DRIVE_H */
