#include "display/gfx.h"
#include "dsp/dsp.h"
#include "gpdma/gpdma.h"
#include "lpc17xx_dac.h"
#include "system/eq_mode.h"
#include "system/system.h"

#include <cstdio>
#include <stdint.h>

/**
 * @brief Frequency-domain bin ranges for each EQ band.
 *
 * ADC sample rate = 32768 Hz, PERIOD = 1024  =>  32 Hz / bin.
 * Unique bins are 0 .. PERIOD/2 - 1 (DC … Nyquist).
 *
 * Band | Hz range   | start_bin | end_bin
 * ----------------------------------------
 * 0    |    0 – 125 |         0 |       3
 * 1    |  125 – 250 |         4 |       7
 * 2    |  250 – 500 |         8 |      15
 * 3    |  500 – 1k  |        16 |      31
 * 4    |  1k – 2k   |        32 |      63
 * 5    |  2k – 4k   |        64 |     127
 * 6    |  4k – 8k   |       128 |     255
 * 7    |  8k – 16k  |       256 |     511
 */
static const uint16_t EQ_BAND_BINS[EQ_BANDS_N][2] = {
    {0, 3},     /**< Band 0:    0 – 125   Hz */
    {4, 7},     /**< Band 1:  125 – 250   Hz */
    {8, 15},    /**< Band 2:  250 – 500   Hz */
    {16, 31},   /**< Band 3:  500 – 1k    Hz */
    {32, 63},   /**< Band 4:  1k – 2k     Hz */
    {64, 127},  /**< Band 5:  2k – 4k     Hz */
    {128, 255}, /**< Band 6:  4k – 8k     Hz */
    {256, 511}, /**< Band 7:  8k – 16k    Hz */
};

static void init(void) {
    init_bars();
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
    case '1': {
        printf("Changed to custom filter \n");
        /* Build one band at a time, mapping eq_bands[i] (0-255) to Q15. */
        for (int i = 0; i < EQ_BANDS_N; i++) {
            int16_t mag = (int16_t)(((int32_t)eq_bands[i] * Q15_ONE) / 255);
            buildFilter(EQ_BAND_BINS[i][0], EQ_BAND_BINS[i][1], mag);
        }
        /* Mirror coefficients to the negative-frequency half (conjugate symmetry). */
        for (int k = 1; k < PERIOD / 2; k++) {
            FILTER_H[PERIOD - k] = FILTER_H[k];
        }
        printf("Build of the FILTER_H for the EQ filter is complete \n");
        break;
    }
    case '2':
        printf("Changed to nosie suppression filter \n");
        /* TODO: Build FILTER_H using the noise suppression mode utils */
        break;
    case '3':
        printf("Changed to passthrough filter \n");
        buildFilter(0, PERIOD - 1, Q15_ONE);
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
