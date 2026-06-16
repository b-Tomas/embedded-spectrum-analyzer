#pragma once

#include "display/gfx.h"

#include <stdint.h>

/**
 * @brief Possible filters that can be applied to modify the signal.
 */
typedef enum {
    passthrough,      /**< No filtering applied. */
    noiseSuppression, /**< Noise-suppression filter (not yet implemented). */
    customEqualized,  /**< User-defined equaliser filter (not yet implemented). */
} Filter;

#define LOG2_PERIOD 10
#define PERIOD      (1 << LOG2_PERIOD)
#define ADC_MAX     4095
#define ADC_CENTER  2048
#define Q15_ONE     32767

/* ---------------------------------------------------------------------------
 * Global DSP buffers
 *
 * These are moved here from gpdma.h because they are managed entirely
 * by CPU-side DSP code (no DMA interaction).  The volatile qualifier is
 * deliberately omitted for performance.
 * ---------------------------------------------------------------------------
 */

/** Real part of the FFT spectrum (also input to IFFT). */
extern int32_t DSP_FFT_RESULT_RE[PERIOD];
/** Imaginary part of the FFT spectrum (also input to IFFT). */
extern int32_t DSP_FFT_RESULT_IM[PERIOD];
/** Time-domain result of the inverse FFT (for DAC). */
extern int32_t DSP_IFFT_RESULT[PERIOD];
/** Frequency-domain filter coefficients (built by buildFilter). */
extern int16_t FILTER_H[PERIOD];
/** Gain parameter for the active filter (set by user input). */
extern int16_t MAGNITUDE;

/**
 * @brief initialize the digital singal processing interface
 */
void dps_init();

/**
 * @brief Compute the in-place FFT of the ADC input.
 *
 * Reads from FFT_SOURCE_BUFFER_TIME (the half indicated by flag_halfReady),
 * centres the samples by subtracting ADC_CENTER, and writes the complex
 * spectrum to DSP_FFT_RESULT_RE / DSP_FFT_RESULT_IM.
 */
void dsp_FFT(void);

/**
 * @brief Compute the inverse FFT.
 *
 * Reads the complex spectrum from DSP_FFT_RESULT_RE / DSP_FFT_RESULT_IM,
 * performs the inverse transform in-place, and stores the real time-domain
 * result (divided by PERIOD) in DSP_IFFT_RESULT.
 */
void dsp_IFFT(void);

/**
 * @brief Build a square filter in FILTER_H.
 *
 * Sets FILTER_H[i] = magnitude for i in [start_bin, end_bin] (inclusive).
 * Bins outside the range are NOT modified — the caller is responsible for
 * zeroing FILTER_H beforehand when constructing a multi-band filter.
 *
 * @param start_bin  First bin to set (0-based).
 * @param end_bin    Last bin to set (inclusive, must be < PERIOD).
 * @param magnitude  Q15 coefficient for the passband (Q15_ONE = passthrough,
 *                   0 = fully blocked).
 */
void buildFilter(uint16_t start_bin, uint16_t end_bin, int16_t magnitude);

/**
 * @brief Multiply the complex spectrum by FILTER_H, in-place.
 *
 * Operates on DSP_FFT_RESULT_RE / DSP_FFT_RESULT_IM.
 */
void applyFilter(void);

/**
 * @brief Compute bar-height data ready for update_bars().
 *
 * @note This function assumes PERIOD = 1024 and N_BARS = 128.
 *       Only the first PERIOD/2 bins are unique (the input is real, so the
 *       upper half is a conjugate mirror). Each bar averages 4 bins.
 *       If these constants change, the averaging logic must be revisited.
 *
 * Steps:
 *   1. Compute per-bin magnitude as abs(re) + abs(im) for bins 0..PERIOD/2-1.
 *   2. Average groups of (PERIOD/2 / N_BARS) bins into N_BARS bars.
 *   3. Dynamically normalise to [0, DISPLAY_HEIGHT].
 *
 * @param bars  Output array of length N_BARS, filled with display heights.
 */
void dsp_computeMagnitudeBars(uint8_t bars[N_BARS]);

/**
 * @brief Select one of the predefined filters.
 * @param filter The new filter to apply.
 */
void dsp_setFilter(Filter filter);

/**
 * @brief Reset the active filter to passthrough.
 */
void dsp_clearFilter(void);
