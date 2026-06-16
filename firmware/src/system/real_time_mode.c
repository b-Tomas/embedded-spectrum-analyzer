#include "gfx.h"
#include "gpdma.h"
#include "system.h"

static void init(void) {
    init_bars();
}

static void deInit() {
    // No-op
}

/* ---------------------------------------------------------------------------
 * Real-time mode execution
 *
 * Called repeatedly from the main-loop state machine.  When the GPDMA
 * completes a PERIOD-sized ADC transfer it sets flag_bufferReadyforFFT;
 * this function consumes it:
 *
 *   1. dsp_FFT()  – transforms the time-domain samples into the frequency
 *                   domain (complex spectrum in DSP_FFT_RESULT_RE/IM).
 *   2. If the display timer has fired (flag_readyToDisplay), produce a
 *      set of bar heights via dsp_computeMagnitudeBars() and update the
 *      OLED with update_bars().
 * ---------------------------------------------------------------------------
 */
static void tick(void) {
    if (!flag_bufferReadyforFFT)
        return;
    flag_bufferReadyforFFT = RESET;

    dsp_FFT();

    if (SYSTEM.flag_readyToDisplay) {
        SYSTEM.flag_readyToDisplay = RESET;

        uint8_t bars[N_BARS];
        dsp_computeMagnitudeBars(bars);
        update_bars(bars);
    }
}

static void handleKey(char c) {
    // No-op
}

static SystemMode_T realTimeModeCfg = {init, deInit, tick, handleKey};

void realTimeMode_registerHooks() {
    system_registerMode(realTimeMode, &realTimeModeCfg);
}
