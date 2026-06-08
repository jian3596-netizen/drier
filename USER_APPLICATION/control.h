#ifndef __CONTROL_H
#define __CONTROL_H

#include "main.h"

/*
 * Closed-loop heat/fan control + safety interlocks for the STM32 control board.
 *
 * Architecture note: the DWIN screen computes temperature (with its own
 * calibration) and sends back: current temps, setpoint, run state, preheat
 * flag. This board runs the actual PI loop and drives the heater/fan PWMs,
 * plus several safety interlocks that force the heater OFF on any fault.
 */

void Control_Init(void);    /* call once after peripherals are up (starts IWDG) */
void Control_Update(void);  /* call every main-loop cycle (~100 ms)             */

#endif /* __CONTROL_H */
