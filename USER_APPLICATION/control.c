#include "control.h"
#include "tim.h"

/* ============================================================================
 *  Two heating zones (matches the current hardware):
 *    Zone 1: heater = TIM16_CH1 (PA6), sensor = temp1, resistance = g_Res[0]
 *    Zone 2: heater = TIM1_CH2  (PA9), sensor = temp2, resistance = g_Res[1]
 *  Both are driven to the same setpoint sent by the screen.
 *
 *  All-integer (no float) so the Cortex-M0 doesn't pull in the soft-float
 *  library. Control period is the fixed ~100 ms main loop.
 *  Tunable parameters live in g_cfg (below): defaults are placeholders and
 *  MUST be tuned / verified on real hardware.
 * ==========================================================================*/
/* Structural constants (not meant to be tuned by temperature). */
#define COMM_TIMEOUT_MS   1000U   /* no valid screen packet within this -> heaters OFF        */
#define FAN_RUN              0U   /* fan PWM while heating (inverted: 0 = full, 100 = off)    */
#define FAN_STOP           100U   /* fan PWM when stopped                                     */
#define ITERM_LIMIT     100000    /* clamp on scaled integral term (= 100% * 1000)           */

/* ---------------------------------------------------------------------------
 *  Bench-tunable configuration (type ControlCfg, declared in control.h).
 *
 *  These are RAM globals (NOT #define) on purpose: you can read AND change
 *  every field LIVE in the J-Link debugger's Watch / Live-Watch window while
 *  the board runs -- no recompile. Edit a field, resume, and it takes effect
 *  on the next ~100 ms loop. This is the embedded equivalent of a config file.
 *  The defaults below are starting points and still need bench tuning.
 *
 *    plate_temp_max : heating-plate sensor (temp3) over-temp cutoff [degC].
 *                     temp3 >= this -> BOTH heaters cut off (fan keeps purging).
 *                     Mirrors the original firmware's safety cutoff; the screen
 *                     raises its own error popup at temp > 128.
 *    hard_temp_max  : per-zone (temp1/temp2) over-temp cutoff [degC].
 *    res_min_ohm    : NTC resistance below this = short -> that zone faults off.
 *    res_max_ohm    : NTC resistance above this = open  -> that zone faults off.
 *    pi_kp          : PI proportional gain, %power per degC of error.
 *    pi_ki_inc      : PI integral increment per cycle = Ki*dt*1000
 *                     (Ki=0.40, dt=0.1s -> 40). Integral term is scaled x1000.
 * ------------------------------------------------------------------------- */
ControlCfg g_cfg = {
    128,      /* plate_temp_max : temp3 >= 128 -> both heaters off (screen alarms at >128) */
    120,      /* hard_temp_max  : per-zone temp1/temp2 over-temp cutoff                     */
    500,      /* res_min_ohm    : NTC short threshold                                       */
    30000,    /* res_max_ohm    : NTC open  threshold                                       */
    12,       /* pi_kp                                                                      */
    40        /* pi_ki_inc      : = Ki*dt*1000 (Ki=0.40, dt=0.1s)                           */
};

/* Latched screen state + per-zone integral term (scaled x1000) */
static uint8_t  s_run     = 0;
static uint8_t  s_preheat = 0;
static uint8_t  s_setpt   = 0;
static uint8_t  s_t1 = 0, s_t2 = 0, s_t3 = 0;
static int32_t  s_iterm[2] = {0, 0};

/* Heater PWM is INVERTED: compare 0 = full power, 99 = OFF. pct in 0..100. */
static void heater_pwm(TIM_HandleTypeDef *htim, uint32_t ch, int32_t pct)
{
    uint16_t cmp;
    if (pct < 0)   pct = 0;
    if (pct > 100) pct = 100;
    cmp = (uint16_t)((100 - pct) * 99 / 100);   /* 100% -> 0 (full), 0% -> 99 (off) */
    __HAL_TIM_SetCompare(htim, ch, cmp);
}

static void heaters_off(void)
{
    __HAL_TIM_SetCompare(&htim16, TIM_CHANNEL_1, 99);   /* zone 1 */
    __HAL_TIM_SetCompare(&htim1,  TIM_CHANNEL_2, 99);   /* zone 2 */
}

static void fans_set(uint16_t duty)
{
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, duty);  /* fan PA7 */
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_4, duty);  /* fan PB1 */
}

/* Open circuit -> huge (or wrapped-huge) resistance; short -> tiny. Either = fault. */
static uint8_t sensor_fault(uint32_t res_ohm)
{
    return (res_ohm < g_cfg.res_min_ohm || res_ohm > g_cfg.res_max_ohm) ? 1 : 0;
}

/* Integer PI (per zone) with clamping anti-windup. Returns 0..100 %. */
static int32_t pi_update(uint8_t z, int32_t setpoint, int32_t meas)
{
    int32_t err = setpoint - meas;
    int32_t out = g_cfg.pi_kp * err + s_iterm[z] / 1000;
    if (out > 0 && out < 100)             /* integrate only when not saturated */
    {
        s_iterm[z] += g_cfg.pi_ki_inc * err;
        if (s_iterm[z] >  ITERM_LIMIT) s_iterm[z] =  ITERM_LIMIT;
        if (s_iterm[z] < -ITERM_LIMIT) s_iterm[z] = -ITERM_LIMIT;
        out = g_cfg.pi_kp * err + s_iterm[z] / 1000;
    }
    if (out > 100) out = 100;
    if (out < 0)   out = 0;
    return out;
}

/* Drive one zone: safety-check its sensor, then PI; force OFF on fault. */
static void zone_update(uint8_t z, TIM_HandleTypeDef *htim, uint32_t ch,
                        uint32_t res, uint8_t temp)
{
    if (sensor_fault(res) || temp >= g_cfg.hard_temp_max)
    {
        __HAL_TIM_SetCompare(htim, ch, 99);   /* OFF */
        s_iterm[z] = 0;
        return;
    }
    heater_pwm(htim, ch, pi_update(z, (int32_t)s_setpt, (int32_t)temp));
}

/* ===========================================================================*/
void Control_Init(void)
{
    /* Keep IWDG from resetting the chip while halted at a debug breakpoint. */
    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_IWDG_STOP;

    /* Independent watchdog, ~3.2 s. LSI ~40 kHz / 64 = ~625 Hz; reload 2000. */
    IWDG->KR  = 0x00005555;   /* unlock PR/RLR */
    IWDG->PR  = 4;            /* /64           */
    IWDG->RLR = 2000;
    IWDG->KR  = 0x0000AAAA;   /* reload */
    IWDG->KR  = 0x0000CCCC;   /* start (also turns LSI on) */

    s_iterm[0] = s_iterm[1] = 0;
}

void Control_Update(void)
{
    uint32_t now;

    IWDG->KR = 0x0000AAAA;   /* feed watchdog: main loop is alive */

    if (st_Uart1.USART_RX_SUCCESS)
    {
        st_Uart1.USART_RX_SUCCESS = 0;
        s_t1      = st_Uart1.a_Rx_Buf[1];
        s_t2      = st_Uart1.a_Rx_Buf[2];
        s_t3      = st_Uart1.a_Rx_Buf[3];
        s_run     = st_Uart1.a_Rx_Buf[4];
        s_setpt   = st_Uart1.a_Rx_Buf[5];
        s_preheat = st_Uart1.a_Rx_Buf[6];
    }

    now = HAL_GetTick();

    /* Safety 1: communication lost -> heaters OFF (keep fan to purge heat). */
    if ((now - g_last_rx_tick) > COMM_TIMEOUT_MS)
    {
        heaters_off();
        fans_set(FAN_RUN);
        s_iterm[0] = s_iterm[1] = 0;
        return;
    }

    /* Run and preheat both control to the setpoint (requirement #3).
       Per-zone over-temp is checked inside zone_update(); the heating-plate
       sensor (temp3) gives a separate whole-unit over-temp backstop below. */
    if (s_run == 1 || s_preheat == 1)
    {
        fans_set(FAN_RUN);                                  /* fan must run while heating */

        /* Plate over-temp backstop (re-added): heating-plate sensor (temp3) too
           hot -> cut BOTH heaters, keep the fan running to purge heat. Protects
           the plate/housing. temp3 comes from the screen; an OPEN plate NTC
           reads ~0 (cold) so this won't false-trip, but it also can't see an
           over-temp while the plate probe is disconnected. Threshold is
           g_cfg.plate_temp_max (live-tunable). */
        if (s_t3 >= g_cfg.plate_temp_max)
        {
            heaters_off();
            s_iterm[0] = s_iterm[1] = 0;
            return;
        }

        zone_update(0, &htim16, TIM_CHANNEL_1, g_Res[0], s_t1);  /* zone 1 */
        zone_update(1, &htim1,  TIM_CHANNEL_2, g_Res[1], s_t2);  /* zone 2 */
    }
    else
    {
        heaters_off();
        fans_set(FAN_STOP);
        s_iterm[0] = s_iterm[1] = 0;
    }
}
