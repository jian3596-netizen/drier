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

/* ---------------------------------------------------------------------------
 * Bench-tunable configuration. Lives in RAM (see g_cfg in control.c) so every
 * field can be inspected AND changed LIVE in the J-Link debugger watch window
 * without recompiling -- the embedded equivalent of a config file.
 * ------------------------------------------------------------------------- */
typedef struct {
    uint8_t  plate_temp_max;  /* heating-plate (temp3) over-temp cutoff, degC   */
    uint8_t  hard_temp_max;   /* per-zone (temp1/temp2) over-temp cutoff, degC  */
    uint32_t res_min_ohm;     /* NTC below this = short -> zone faults off       */
    uint32_t res_max_ohm;     /* NTC above this = open  -> zone faults off       */
    int32_t  pi_kp;           /* PI proportional gain (%power per degC error)    */
    int32_t  pi_ki_inc;       /* PI integral increment per cycle (Ki*dt*1000)    */
} ControlCfg;

extern ControlCfg g_cfg;      /* defaults in control.c; live-tunable in debugger */

void Control_Init(void);    /* call once after peripherals are up (starts IWDG) */
void Control_Update(void);  /* call every main-loop cycle (~100 ms)             */

#endif /* __CONTROL_H */
