#include "dsp/dsp.h"

#include "display/SSD1306.h"
#include "display/display.h"
#include "display/gfx.h"
#include "dsp/fft_tables.h"
#include "gpdma/gpdma.h"
#include "system/system.h"

#include <stdint.h>
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
 * Reads FFT_SOURCE_BUFFER_TIME (the half indicated by fft_half_ready),
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
     * fft_half_ready is toggled by the CH7 TC interrupt; a read of a
     * volatile int is atomic on Cortex-M3.
     */
    int half = fft_half_ready;
    uint16_t* src = (uint16_t*)FFT_SOURCE_BUFFER_TIME + (half * PERIOD);

    /* Load and centre the time-domain samples. */
    for (int i = 0; i < PERIOD; i++) {
        DSP_FFT_RESULT_RE[i] = (int32_t)src[i] - ADC_CENTER;
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
 * Inspects SYSTEM.filter and fills FILTER_H accordingly.
 *
 * Currently only passthrough is implemented:
 *   - All bins are set to Q15_ONE so that applyFilter becomes a no-op.
 *
 * noiseSuppression and customEqualized are reserved.
 * ---------------------------------------------------------------------------
 */
void buildFilter(void) {
    switch (SYSTEM.filter) {
    case passthrough:
        for (int i = 0; i < PERIOD; i++) {
            FILTER_H[i] = Q15_ONE;
        }
        break;

    case noiseSuppression:
        break;
    case customEqualized:
        /** TODO: implement filter configuration for these modes. */
        break;
    }
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
 * Algorithm:
 *   1. If the active filter is not passthrough, build and apply it.
 *   2. Magnitude per bin = |re| + |im|  (avoids expensive sqrt).
 *   3. Group PERIOD bins into N_BARS averages (base = PERIOD / N_BARS = 8).
 *   4. Dynamically normalise the bar heights to [0, DISPLAY_HEIGHT].
 * ---------------------------------------------------------------------------
 */
void dsp_computeMagnitudeBars(uint8_t bars[N_BARS]) {
    if (SYSTEM.filter != passthrough) {
        buildFilter();
        applyFilter();
    }

    /* ---- Per-bin magnitude approximation ---- */
    int32_t mag[PERIOD];
    for (int i = 0; i < PERIOD; i++) {
        int32_t re = DSP_FFT_RESULT_RE[i];
        int32_t im = DSP_FFT_RESULT_IM[i];
        mag[i] = (re < 0 ? -re : re) + (im < 0 ? -im : im);
    }

    /* ---- Bin averaging (PERIOD / N_BARS = 8 samples per bar) ---- */
    int32_t barSums[N_BARS];
    int base = PERIOD / N_BARS; /* 1024 / 128 = 8 */
    int idx = 0;

    for (int bar = 0; bar < N_BARS; bar++) {
        int32_t sum = 0;
        for (int j = 0; j < base; j++) {
            sum += mag[idx++];
        }
        barSums[bar] = sum / base;
    }

    /* ---- Dynamic normalisation to [0, DISPLAY_HEIGHT] ---- */
    int32_t maxAvg = 0;
    for (int bar = 0; bar < N_BARS; bar++) {
        if (barSums[bar] > maxAvg) {
            maxAvg = barSums[bar];
        }
    }

    if (maxAvg > 0) {
        for (int bar = 0; bar < N_BARS; bar++) {
            uint8_t v = (uint8_t)((barSums[bar] * DISPLAY_HEIGHT) / maxAvg);
            bars[bar] = (v > DISPLAY_HEIGHT) ? DISPLAY_HEIGHT : v;
        }
    } else {
        memset(bars, 0, N_BARS * sizeof(uint8_t));
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
