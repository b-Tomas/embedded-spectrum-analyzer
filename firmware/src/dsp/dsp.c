#include "dsp/dsp.h"

#include "display/SSD1306.h"
#include "display/display.h"
#include "display/gfx.h"
#include "dsp/fft_tables.h"
#include "gpdma/gpdma.h"
#include "system/system.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* ---------------------------------------------------------------------------
 * Global DSP buffer definitions
 *
 * Declared extern in dsp.h; instantiated here so they live in the DSP
 * translation unit.  No volatile qualifier because all access is CPU-side.
 * ---------------------------------------------------------------------------
 */
int32_t DSP_FFT_RESULT_RE[PERIOD];
int32_t DSP_FFT_RESULT_IM[PERIOD];
int32_t DSP_IFFT_RESULT[PERIOD];
int16_t FILTER_H[PERIOD];
int16_t MAGNITUDE;

void dsp_init() {
    buildFilter(0, PERIOD - 1, Q15_ONE);
    dsp_setFilter(passthrough);
}
/* ---------------------------------------------------------------------------
 * Q15 multiplication helper
 *
 * Performs (a * b) >> 15 with rounding.  The ARM compiler emits a single
 * SMULL instruction for the wide multiply.
 *
 * With an input amplitude of 2047 and N = 1024, the worst-case FFT
 * accumulation is 2047 * 1024 ≈ 2.1e6, well within int32_t range.
 * ---------------------------------------------------------------------------
 */
static inline int32_t mul_q15(int32_t a, int16_t b) {
    return (int32_t)(((int64_t)a * (int32_t)b + 16384) >> 15);
}

/* ---------------------------------------------------------------------------
 * Fixed-point log2 helper
 *
 * Returns floor(log2(x)) plus a fractional part in Q.LOG2_FRAC_BITS fixed
 * point, or 0 for x <= 0.  __builtin_clz compiles to a single CLZ on the
 * Cortex-M3; the fractional term linearly interpolates within each octave so
 * the bar heights vary smoothly instead of jumping a whole pixel band per
 * doubling of magnitude.
 * ---------------------------------------------------------------------------
 */
#define LOG2_FRAC_BITS 4 /* fractional resolution within one octave (Q.4) */

static inline int32_t log2_fixed(int32_t x) {
    if (x <= 0)
        return 0;
    int e = 31 - __builtin_clz((uint32_t)x);                /* floor(log2(x)) */
    int32_t frac = ((x - (1 << e)) << LOG2_FRAC_BITS) >> e; /* (x-2^e)/2^e    */
    return ((int32_t)e << LOG2_FRAC_BITS) + frac;
}

/* ---------------------------------------------------------------------------
 * In-place bit-reversal permutation
 *
 * Uses the precomputed bit-reversal table from fft_tables.c.
 * ---------------------------------------------------------------------------
 */
static void bit_reverse(int32_t* re, int32_t* im) {
    for (int i = 0; i < PERIOD; i++) {
        unsigned int r = bit_rev_table[i];
        if ((int)r > i) {
            int32_t t;
            t = re[i];
            re[i] = re[r];
            re[r] = t;
            t = im[i];
            im[i] = im[r];
            im[r] = t;
        }
    }
}

/* ---------------------------------------------------------------------------
 * dsp_FFT
 *
 * Reads FFT_SOURCE_BUFFER_TIME (the half indicated by flag_halfReady),
 * centres the signal by subtracting ADC_CENTER, performs a radix-2 DIT FFT
 * in-place on the global DSP_FFT_RESULT_RE / DSP_FFT_RESULT_IM arrays,
 * and writes the complex spectrum back to the same arrays.
 *
 * No internal scaling is applied; the twiddle factors are Q15.
 * ---------------------------------------------------------------------------
 */
void dsp_FFT(void) {
    /*
     * Determine which half of the double buffer is ready.
     * flag_halfReady is toggled by the CH7 TC interrupt; a read of a
     * volatile int is atomic on Cortex-M3.
     */
    int half = flag_halfReady;
    volatile uint16_t* src = FFT_SOURCE_BUFFER_TIME + (half * PERIOD);

    /* Load and centre the time-domain samples. */
    for (int i = 0; i < PERIOD; i++) {
        DSP_FFT_RESULT_RE[i] = (int32_t)(src[i] >> 4) - ADC_CENTER;
        DSP_FFT_RESULT_IM[i] = 0;
    }

    bit_reverse(DSP_FFT_RESULT_RE, DSP_FFT_RESULT_IM);

    /* Radix-2 decimation-in-time FFT, 10 stages. */
    for (int stage = 0; stage < LOG2_PERIOD; stage++) {
        int len = 1 << (stage + 1);
        int half = len >> 1;
        int step = PERIOD / len;

        for (int k = 0; k < PERIOD; k += len) {
            int tw_idx = 0;
            for (int j = 0; j < half; j++, tw_idx += step) {
                /*
                 * wr =  cos(2*pi * tw_idx / N)
                 * wi = -sin(2*pi * tw_idx / N)
                 *
                 * sin(x) = cos(x - pi/2), so with tw_cos indexed by
                 * (tw_idx + 3*N/4) & (N-1) we get cos(tw_idx - pi/2)
                 * = sin(tw_idx) negated.
                 */
                int16_t wr = tw_cos[tw_idx];
                int16_t wi = -tw_cos[(tw_idx + 3 * PERIOD / 4) & (PERIOD - 1)];

                int32_t ur = DSP_FFT_RESULT_RE[k + j];
                int32_t ui = DSP_FFT_RESULT_IM[k + j];
                int32_t vr = DSP_FFT_RESULT_RE[k + j + half];
                int32_t vi = DSP_FFT_RESULT_IM[k + j + half];

                int32_t tr = mul_q15(vr, wr) - mul_q15(vi, wi);
                int32_t ti = mul_q15(vr, wi) + mul_q15(vi, wr);

                DSP_FFT_RESULT_RE[k + j] = ur + tr;
                DSP_FFT_RESULT_IM[k + j] = ui + ti;
                DSP_FFT_RESULT_RE[k + j + half] = ur - tr;
                DSP_FFT_RESULT_IM[k + j + half] = ui - ti;
            }
        }
    }

    /* Result is already in DSP_FFT_RESULT_RE / DSP_FFT_RESULT_IM – no memcpy. */
}

/* ---------------------------------------------------------------------------
 * dsp_IFFT
 *
 * Reads the complex spectrum from DSP_FFT_RESULT_RE / DSP_FFT_RESULT_IM,
 * performs a radix-2 inverse FFT in-place (using conjugate twiddle factors),
 * divides by PERIOD, and writes the real time-domain result to
 * DSP_IFFT_RESULT.
 *
 * The input arrays are modified during processing; call this only after
 * dsp_computeMagnitudeBars has consumed the data for display.
 * ---------------------------------------------------------------------------
 */
void dsp_IFFT(void) {
    bit_reverse(DSP_FFT_RESULT_RE, DSP_FFT_RESULT_IM);

    for (int stage = 0; stage < LOG2_PERIOD; stage++) {
        int len = 1 << (stage + 1);
        int half = len >> 1;
        int step = PERIOD / len;

        for (int k = 0; k < PERIOD; k += len) {
            int tw_idx = 0;
            for (int j = 0; j < half; j++, tw_idx += step) {
                /*
                 * wr =  cos(2*pi * tw_idx / N)
                 * wi = +sin(2*pi * tw_idx / N)  – positive sign for IFFT.
                 */
                int16_t wr = tw_cos[tw_idx];
                int16_t wi = tw_cos[(tw_idx + 3 * PERIOD / 4) & (PERIOD - 1)];

                int32_t ur = DSP_FFT_RESULT_RE[k + j];
                int32_t ui = DSP_FFT_RESULT_IM[k + j];
                int32_t vr = DSP_FFT_RESULT_RE[k + j + half];
                int32_t vi = DSP_FFT_RESULT_IM[k + j + half];

                int32_t tr = mul_q15(vr, wr) - mul_q15(vi, wi);
                int32_t ti = mul_q15(vr, wi) + mul_q15(vi, wr);

                DSP_FFT_RESULT_RE[k + j] = ur + tr;
                DSP_FFT_RESULT_IM[k + j] = ui + ti;
                DSP_FFT_RESULT_RE[k + j + half] = ur - tr;
                DSP_FFT_RESULT_IM[k + j + half] = ui - ti;
            }
        }
    }

    /* Divide by N and keep only the real part. */
    for (int k = 0; k < PERIOD; k++) {
        DSP_IFFT_RESULT[k] = DSP_FFT_RESULT_RE[k] / PERIOD;
    }
}

/* ---------------------------------------------------------------------------
 * buildFilter
 *
 * Sets FILTER_H[i] = magnitude for i in [start_bin, end_bin] (inclusive).
 * Bins outside the range are left unchanged.
 * ---------------------------------------------------------------------------
 */
void buildFilter(uint16_t start_bin, uint16_t end_bin, int16_t magnitude) {
    for (uint16_t i = start_bin; i <= end_bin; i++) {
        FILTER_H[i] = magnitude;
    }
}

void dsp_buildEqualizationFilter(void) {

    if (SYSTEM.filter == customEqualized) {
        printf("Already in EQ filter \n");
        return;
    }
    dsp_setFilter(customEqualized);

    printf("Changed to EQ filter \n");
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
}

void dsp_buildNoiseSuppressionFilter(void) {
    if (SYSTEM.filter == noiseSuppression) {
        printf("Already in noise suppression filter");
        return;
    }
    dsp_setFilter(noiseSuppression);

    printf("Changed to nosie suppression filter \n");
    /* TODO: Build FILTER_H using the noise suppression mode utils */
}

void dsp_buildPassthroughFilterd(void) {
    if (SYSTEM.filter == passthrough) {
        printf("Already in passtrough");
        return;
    }
    dsp_setFilter(passthrough);

    printf("Changed to passthrough filter \n");
    buildFilter(0, PERIOD - 1, Q15_ONE);
    printf("Build of the FILTER_H passthrough filter is complete \n");
}

/* ---------------------------------------------------------------------------
 * applyFilter
 *
 * Multiplies the complex spectrum (DSP_FFT_RESULT_RE + j * DSP_FFT_RESULT_IM)
 * by the real filter FILTER_H, in-place.  The multiplication uses Q15
 * arithmetic; buildFilter guarantees that all coefficients are within the
 * valid Q15 range, so no clamping is required.
 * ---------------------------------------------------------------------------
 */
void applyFilter(void) {
    for (int i = 0; i < PERIOD; i++) {
        DSP_FFT_RESULT_RE[i] = mul_q15(DSP_FFT_RESULT_RE[i], FILTER_H[i]);
        DSP_FFT_RESULT_IM[i] = mul_q15(DSP_FFT_RESULT_IM[i], FILTER_H[i]);
    }
}

/* ---------------------------------------------------------------------------
 * dsp_computeMagnitudeBars
 *
 * Produces a uint8_t array suitable for update_bars() from the complex
 * FFT spectrum.
 *
 * NOTE: This function assumes PERIOD = 1024 and N_BARS = 128.
 *       Only the first PERIOD/2 bins are unique (the input is real, so the
 *       upper half is a conjugate mirror). Each bar averages 4 bins.
 *       If these constants change, the averaging logic must be revisited.
 *
 * Algorithm:
 *   1. Magnitude per bin = |re| + |im|  (avoids expensive sqrt).
 *   2. Group PERIOD/2 bins into N_BARS averages (base = 4).
 *   3. Map each average to [0, DISPLAY_HEIGHT] with a fixed logarithmic
 *      (dB-style) scale, so bar height tracks absolute amplitude rather than
 *      being renormalised per frame.
 *
 * NOTE: applyFilter() must be called before this function if a non-trivial
 *       filter is active.
 * ---------------------------------------------------------------------------
 */
#define BAR_LOG2_FLOOR    6 /* magnitudes <= 2^6 (=64) render as an empty bar */
#define BAR_PX_PER_OCTAVE 5 /* display pixels per doubling of magnitude        */

void dsp_computeMagnitudeBars(uint8_t bars[N_BARS]) {
    int const nBins = PERIOD / 2;    /* bins 0..511 are unique */
    int const base = nBins / N_BARS; /* 512 / 128 = 4 */

    /* ---- Per-bin magnitude approximation (unique half only) ---- */
    int32_t mag[nBins];
    for (int i = 0; i < nBins; i++) {
        int32_t re = DSP_FFT_RESULT_RE[i];
        int32_t im = DSP_FFT_RESULT_IM[i];
        mag[i] = (re < 0 ? -re : re) + (im < 0 ? -im : im);
    }

    /* ---- Bin averaging (base = 4 samples per bar) ---- */
    int32_t barSums[N_BARS];
    int idx = 0;

    for (int bar = 0; bar < N_BARS; bar++) {
        int32_t sum = 0;
        for (int j = 0; j < base; j++) {
            sum += mag[idx++];
        }
        barSums[bar] = sum / base;
    }

    /* ---- Fixed logarithmic mapping to [0, DISPLAY_HEIGHT] ---- */
    for (int bar = 0; bar < N_BARS; bar++) {
        int32_t l = log2_fixed(barSums[bar]);
        int32_t px =
            ((l - (BAR_LOG2_FLOOR << LOG2_FRAC_BITS)) * BAR_PX_PER_OCTAVE) >> LOG2_FRAC_BITS;
        if (px < 0)
            px = 0;
        if (px > DISPLAY_HEIGHT)
            px = DISPLAY_HEIGHT;
        bars[bar] = (uint8_t)px;
    }
}

/* ---------------------------------------------------------------------------
 * Filter selection helpers
 * ---------------------------------------------------------------------------
 */
void dsp_setFilter(Filter filter) {
    SYSTEM.filter = filter;
}

void dsp_clearFilter(void) {
    dsp_setFilter(passthrough);
}
