#include "system/system.h"

#include "LPC17xx.h"
#include "adc/adc.h"
#include "display/display.h"
#include "display/gfx.h"
#include "dsp/dsp.h"
#include "gpdma/gpdma.h"
#include "input/keyboard.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"
#include "lpc_types.h"

#include <stddef.h>
#include <stdint.h>

/** @brief Global orchestrator instance. */
System SYSTEM;

FlagStatus flag_readyToDisplay = RESET;

void system_Init(Mode mode) {
    system_setMode(mode);
    display_init();
    init_bars();
    adc_init();
    gpdma_init();
    DAC_Init();
    kbd_init();
}

void system_ConfigureSetting_RealTimeMode(void) {
    /** TODO: Display behaviour */
}

void system_ConfigureSetting_NoiseSamplingMode(void) {
    /* TODO: ADC / GPDMA / display reconfiguration. */
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
void system_tickRealTimeMode(void) {
    if (!flag_bufferReadyforFFT)
        return;
    flag_bufferReadyforFFT = RESET;

    dsp_FFT();

    if (flag_readyToDisplay) {
        flag_readyToDisplay = RESET;

        uint8_t bars[N_BARS];
        dsp_computeMagnitudeBars(bars);
        update_bars(bars);
    }
}

void system_tickNoiseSamplingMode(void) {
    /** TODO: Noise-sampling mode implementation. */
}

void system_setMode(Mode mode) {
    SYSTEM.mode = mode;
    SYSTEM.flag_ModeConfigured = RESET;
}
