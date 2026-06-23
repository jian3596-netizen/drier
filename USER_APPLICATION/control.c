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
 *    preheat_timeout_min : preheat auto-stops this many minutes after the user
 *                     switches it on (default 60). Live-tunable -- set it to 1
 *                     in the debugger to test the auto-stop quickly.
 * ------------------------------------------------------------------------- */
ControlCfg g_cfg = {
    128,      /* plate_temp_max : temp3 >= 128 -> both heaters off (screen alarms at >128) */
    120,      /* hard_temp_max  : per-zone temp1/temp2 over-temp cutoff                     */
    500,      /* res_min_ohm    : NTC short threshold                                       */
    250000,   /* res_max_ohm    : NTC open threshold. MEASURED: healthy NTC ~46k@room,      */
              /*                  ~163k@0C; a true open reads ~367k. The old 30000 was WAY  */
              /*                  too low -> a cold/room-temp probe looked "open" and BOTH  */
              /*                  zones were force-OFF (the "won't heat from cold" bug).    */
    12,       /* pi_kp                                                                      */
    40,       /* pi_ki_inc      : = Ki*dt*1000 (Ki=0.40, dt=0.1s)                           */
    60        /* preheat_timeout_min : preheat auto-stops 60 min after switched on          */
};

/* Latched screen state + per-zone integral term (scaled x1000) */
static uint8_t  s_run     = 0;
static uint8_t  s_preheat = 0;
static uint8_t  s_setpt   = 0;
static uint8_t  s_t1 = 0, s_t2 = 0, s_t3 = 0;
static int32_t  s_iterm[2] = {0, 0};

/* Preheat ownership: s_preheat is the raw screen flag; s_preheat_active is the
   effective state THIS board controls, so preheat (a) defaults OFF at every
   boot even if the screen flag is already on, and (b) auto-stops on a timer. */
static uint8_t  s_preheat_active = 0;
static uint8_t  s_preheat_seen   = 0;   /* first screen packet adopted yet?      */
static uint32_t s_preheat_start  = 0;   /* HAL tick when preheat was switched on  */

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
        uint8_t pre;
        st_Uart1.USART_RX_SUCCESS = 0;
        s_t1      = st_Uart1.a_Rx_Buf[1];
        s_t2      = st_Uart1.a_Rx_Buf[2];
        s_t3      = st_Uart1.a_Rx_Buf[3];
        s_run     = st_Uart1.a_Rx_Buf[4];
        s_setpt   = st_Uart1.a_Rx_Buf[5];

        /* Preheat: activate the effective state ONLY on a fresh OFF->ON edge of
           the screen flag AFTER boot. The first packet is just adopted (no
           activation), so a flag left ON across a reboot will NOT auto-start
           preheat -> "every power-on defaults to preheat OFF". */
        pre = st_Uart1.a_Rx_Buf[6];
        if (!s_preheat_seen)            s_preheat_seen = 1;
        else if (pre && !s_preheat)   { s_preheat_active = 1; s_preheat_start = HAL_GetTick(); }
        else if (!pre)                  s_preheat_active = 0;
        s_preheat = pre;
    }

    now = HAL_GetTick();

    /* Preheat auto-stop: switch it off once it has been on for the configured
       time (default 60 min). After this the user must toggle preheat off->on on
       the screen to restart it. */
    if (s_preheat_active &&
        (now - s_preheat_start) >= (uint32_t)g_cfg.preheat_timeout_min * 60000U)
    {
        s_preheat_active = 0;
    }

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
    if (s_run == 1 || s_preheat_active == 1)
    {
        fans_set(FAN_RUN);                                  /* fan must run while heating */

        /* Plate over-temp backstop (re-added): heating-plate sensor (temp3) too
           hot -> cut BOTH heaters, fan keeps purging. Protects the plate/housing.

           FAULT GUARD (important): temp3 is the screen's reading of the plate
           NTC on PA4 (raw resistance g_Res[2]). If that channel shorts / floats
           high / has a bad contact, ADC_DRIVE forces g_Res[2]=0 and the screen
           then reports temp3=129 -- which would FALSE-TRIP this cut and silently
           block ALL heating every cycle (and temp3 is a background value, not
           shown on screen, so the fault is invisible). So only believe an
           over-temp when the raw plate resistance is NON-ZERO (= a real,
           connected probe). A genuinely hot plate still reads a small but
           non-zero resistance, so real over-temp still trips. (A true-open plate
           reads temp3~0 and never trips anyway.) plate_temp_max is live-tunable.
           NOTE: a hard-shorted/dead plate NTC (g_Res[2] stuck at 0) thus runs
           WITHOUT plate over-temp protection -- per-zone over-temp + watchdog +
           the recommended independent hardware cutoff are the backstops then. */
        if (g_Res[2] != 0 && s_t3 >= g_cfg.plate_temp_max)
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
