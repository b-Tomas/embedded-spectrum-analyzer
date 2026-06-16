#include "display/gfx.h"
#include "dsp/dsp.h"
#include "gpdma/gpdma.h"
#include "lpc17xx_dac.h"
#include "system/eq_mode.h"
#include "system/system.h"

#include <stdint.h>
#include <stdio.h>

static void init(void) {
    init_bars();
    /** rebuild the filter if it was not in passthrough (only on change mode)*/
    if (SYSTEM.filter == customEqualized) {
        dsp_setFilter(passthrough);
        system_handleKey('1');
    } else if (SYSTEM.filter == noiseSuppression) {
        dsp_setFilter(passthrough);
        system_handleKey('2');
    }
}

static void deInit() {
    /* No-op */
}

/* ---------------------------------------------------------------------------
 * Real-time mode execution
 *
 * Called repeatedly from the main-loop state machine.  When the GPDMA
 * completes a PERIOD-sized ADC transfer it sets flag_bufferReadyforFFT;
 * this function consumes it:
 *
 *   1. dsp_FFT()      – transform time-domain samples into frequency domain.
 *   2. applyFilter()  – multiply the spectrum by FILTER_H (passthrough if
 *                       all coefficients are Q15_ONE).
 *   3. If the display timer has fired, produce bar heights via
 *      dsp_computeMagnitudeBars() and update the OLED.
 *   4. dsp_IFFT()     – transform back to time domain for the DAC.
 * ---------------------------------------------------------------------------
 */
static void tick(void) {
    if (!flag_bufferReadyforFFT)
        return;
    flag_bufferReadyforFFT = RESET;

    dsp_FFT();
    applyFilter();

    if (SYSTEM.flag_readyToDisplay) {
        SYSTEM.flag_readyToDisplay = RESET;

        uint8_t bars[N_BARS];
        dsp_computeMagnitudeBars(bars);
        update_bars(bars);
    }
    dsp_IFFT();
    /** TODO: DAC format and output */
}

static void handleKey(char c) {
    switch (c) {
    case '1':
        dsp_buildEqualizationFilter();
        break;

    case '2':
        dsp_buildNoiseSuppressionFilter();
        break;

    case '3':
        dsp_buildPassthroughFilterd();
        break;

    default:
        printf("system_processKeyRealTimeModekey: Tecla=%c no hace nada bro\n");
        break;
    }
}

static SystemMode_T realTimeModeCfg = {
    init,
    deInit,
    tick,
    handleKey,
};

void realTimeMode_registerHooks() {
    system_registerMode(realTimeMode, &realTimeModeCfg);
}
