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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/** @brief Global orchestrator instance. */
System SYSTEM;

FlagStatus flag_readyToDisplay        = RESET;
FlagStatus flag_noiseSamplingDone     = RESET;
FlagStatus flag_SamplingCooldownReady = RESET;

<<<<<<< Updated upstream
void system_Init(Mode const mode) {
=======
/* ---------------------------------------------------------------------------
 * System initialisation
 * ---------------------------------------------------------------------------
 */
void system_Init(Mode mode) {
>>>>>>> Stashed changes
    system_setMode(mode);
    display_init();
    adc_init();
    gpdma_init();
    DAC_Init();
    kbd_init();
}

/* ---------------------------------------------------------------------------
 * Mode configuration
 * ---------------------------------------------------------------------------
 */
void system_ConfigureSetting_RealTimeMode(void) {
<<<<<<< Updated upstream
    init_bars();
=======
    /* TODO: Display behaviour */
>>>>>>> Stashed changes
}

void system_ConfigureSetting_NoiseSamplingMode(void) {
    /* Configure TIM0 to raise flag_SamplingCooldownReady at a fixed interval. */
    TIM_TIMERCFG_T cfgTIM0 = {TIM_US, 100};
    TIM_MATCHCFG_T cfgMAT0 = {
        TIM_MATCH_0,
        ENABLE,
        DISABLE,
        ENABLE,
        TIM_NOTHING,
        COOLDOWN_TICKS
    };
    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &cfgTIM0);
    TIM_ConfigMatch(LPC_TIM0, &cfgMAT0);
    TIM_Cmd(LPC_TIM0, ENABLE);
    NVIC_EnableIRQ(TIMER0_IRQn);
}

/* ---------------------------------------------------------------------------
 * Real-time mode execution
 *
 * Called repeatedly from the main-loop state machine. When the GPDMA
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

/* ---------------------------------------------------------------------------
 * Noise sampling mode execution
 *
 * Three sequential stages, each guarded by state flags:
 *
 *   Stage 1  – Accumulate N_SAMPLES ADC buffers into NOISE_SAMPLES[].
 *   Stage 1b – Average element-by-element (divide by N_SAMPLES).
 *              Sets flag_noiseSamplingDone when finished.
 *
 *   Stage 2  – Inject the averaged time-domain signal into the DMA source
 *              buffer, run dsp_FFT(), compute per-bin magnitude, derive a
 *              binary mask: 0 where noise dominates (above average magnitude),
 *              Q15_ONE elsewhere. Stores the mask back in NOISE_SAMPLES[].
 *              Sets filtered = true when finished.
 *
 *   Stage 3  – Nothing left to do; return immediately.
 * ---------------------------------------------------------------------------
 */
void system_tickNoiseSamplingMode(void) {
    static uint8_t  sampleCount = 0;
    static uint16_t bufferIndex = 0;
    static bool     filtered    = false;

    /* Stage 3: noise mask is ready, nothing left to do. */
    if (flag_noiseSamplingDone && filtered)
        return;

    /* Stage 2: time-domain average ready → FFT → build binary mask. */
    if (flag_noiseSamplingDone && !filtered) {
        if (!flag_bufferReadyforFFT)
            return;
        flag_bufferReadyforFFT = RESET;

        /* Inject the averaged signal into the buffer read by dsp_FFT().
         * dsp_FFT() subtracts ADC_CENTER internally, so we add it back here
         * so the net input to the FFT is exactly NOISE_SAMPLES[i]. */
        volatile uint16_t *src = FFT_SOURCE_BUFFER_TIME + (flag_halfReady * PERIOD);
        for (int i = 0; i < PERIOD; i++) {
            src[i] = (uint16_t)(NOISE_SAMPLES[i] + ADC_CENTER);
        }

        dsp_FFT();

        /* Compute per-bin magnitude (|re| + |im|) and the global average. */
        int32_t mag[PERIOD];
        int32_t magSum = 0;
        for (int i = 0; i < PERIOD; i++) {
            int32_t re = DSP_FFT_RESULT_RE[i];
            int32_t im = DSP_FFT_RESULT_IM[i];
            mag[i]  = (re < 0 ? -re : re) + (im < 0 ? -im : im);
            magSum += mag[i];
        }
        int32_t magAvg = magSum / PERIOD;

        /* Binary mask: 0 where noise dominates (above average), Q15_ONE elsewhere. */
        for (int i = 0; i < PERIOD; i++) {
            NOISE_SAMPLES[i] = (mag[i] > magAvg) ? 0 : Q15_ONE;
        }

        filtered = true;
        return;
    }

    /* Stage 1: accumulate N_SAMPLES complete buffers. */
    if (sampleCount < N_SAMPLES) {
        if (bufferIndex < PERIOD) {
            NOISE_SAMPLES[bufferIndex] += ADC_GlobalGetData();
            bufferIndex++;
        } else {
            bufferIndex = 0;
            sampleCount++;
        }

    /* Stage 1b: element-wise average. */
    } else {
        if (bufferIndex < PERIOD) {
            NOISE_SAMPLES[bufferIndex] /= N_SAMPLES;
            bufferIndex++;
        } else {
            flag_noiseSamplingDone = SET;
        }
    }
}

<<<<<<< Updated upstream
void system_setMode(Mode const mode) {
=======
/* ---------------------------------------------------------------------------
 * Equalizer mode execution
 * ---------------------------------------------------------------------------
 */
void system_tickEqualizerMode(void) {
    /* TODO: Equaliser mode implementation. */
}

/* ---------------------------------------------------------------------------
 * Mode setter
 * ---------------------------------------------------------------------------
 */
void system_setMode(Mode mode) {
>>>>>>> Stashed changes
    SYSTEM.mode = mode;
    SYSTEM.flag_ModeConfigured = RESET;
}