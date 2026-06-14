#include "fft/fft.h"

#include "fft/fft_tables.h"

#include <stdint.h>
#include <string.h>

// ─── Buffers internos ─────────────────────────────────────────────────────────
static int32_t fft_re[PERIOD];
static int32_t fft_im[PERIOD];

// ─── mul Q15 ──────────────────────────────────────────────────────────────────
// El compilador ARM emite SMULL para esta operación.
// Con señal de amplitud 2047 y N=1024, el máximo acumulado en FFT es
// 2047 * 1024 ≈ 2e6, muy por debajo del límite de int32_t (2.1e9).
static inline int32_t mul_q15(int32_t a, int16_t b) {
    return (int32_t)(((int64_t)a * (int32_t)b + 16384) >> 15);
}

// ─── Bit-reverse in-place ─────────────────────────────────────────────────────
// Usa la tabla precomputada: un acceso a flash en lugar de 10 shifts/OR.
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

// ─── FFT ──────────────────────────────────────────────────────────────────────
// Convención estándar: sin escalado interno.
// Con ADC de 12 bits (máx 2047 centrado) y N=1024, el peor caso es
// 2047 * 1024 ≈ 2.1e6, que cabe holgadamente en int32_t (límite ≈ 2.1e9).
//
// Optimizaciones:
//  - tw_sin[] eliminado; se usa tw_cos[(idx + 768) & 1023] = sin(idx)
//  - tw_idx calculado con acumulador (elimina ~5120 multiplicaciones)
void FFT(uint16_t* sourceT, int32_t* sourceR, int32_t* sourceI) {
    for (int i = 0; i < PERIOD; i++) {
        fft_re[i] = (int32_t)sourceT[i] - ADC_CENTER;
        fft_im[i] = 0;
    }
    bit_reverse(fft_re, fft_im);

    for (int stage = 0; stage < LOG2_PERIOD; stage++) {
        int len = 1 << (stage + 1);
        int half = len >> 1;
        int step = PERIOD / len;

        for (int k = 0; k < PERIOD; k += len) {
            int tw_idx = 0;
            for (int j = 0; j < half; j++, tw_idx += step) {
                // wr = cos(2π·tw_idx/N),  wi = −sin(2π·tw_idx/N)
                // sin(x) = cos(x − π/2)  →  índice offset = 3·N/4 = 768
                int16_t wr = tw_cos[tw_idx];
                int16_t wi = -tw_cos[(tw_idx + 3 * PERIOD / 4) & (PERIOD - 1)];

                int32_t ur = fft_re[k + j];
                int32_t ui = fft_im[k + j];
                int32_t vr = fft_re[k + j + half];
                int32_t vi = fft_im[k + j + half];

                int32_t tr = mul_q15(vr, wr) - mul_q15(vi, wi);
                int32_t ti = mul_q15(vr, wi) + mul_q15(vi, wr);

                fft_re[k + j] = ur + tr;
                fft_im[k + j] = ui + ti;
                fft_re[k + j + half] = ur - tr;
                fft_im[k + j + half] = ui - ti;
            }
        }
    }

    memcpy(sourceR, fft_re, PERIOD * sizeof(int32_t));
    memcpy(sourceI, fft_im, PERIOD * sizeof(int32_t));
}

// ─── IFFT ─────────────────────────────────────────────────────────────────────
// Sin escalado interno; divide por PERIOD al final (convención estándar).
void IFFT(int32_t* sourceR, int32_t* sourceI, int32_t* resultT) {
    memcpy(fft_re, sourceR, PERIOD * sizeof(int32_t));
    memcpy(fft_im, sourceI, PERIOD * sizeof(int32_t));
    bit_reverse(fft_re, fft_im);

    for (int stage = 0; stage < LOG2_PERIOD; stage++) {
        int len = 1 << (stage + 1);
        int half = len >> 1;
        int step = PERIOD / len;

        for (int k = 0; k < PERIOD; k += len) {
            int tw_idx = 0;
            for (int j = 0; j < half; j++, tw_idx += step) {
                // wr = cos(2π·tw_idx/N),  wi = +sin(2π·tw_idx/N)  (signo IFFT)
                int16_t wr = tw_cos[tw_idx];
                int16_t wi = tw_cos[(tw_idx + 3 * PERIOD / 4) & (PERIOD - 1)];

                int32_t ur = fft_re[k + j];
                int32_t ui = fft_im[k + j];
                int32_t vr = fft_re[k + j + half];
                int32_t vi = fft_im[k + j + half];

                int32_t tr = mul_q15(vr, wr) - mul_q15(vi, wi);
                int32_t ti = mul_q15(vr, wi) + mul_q15(vi, wr);

                fft_re[k + j] = ur + tr;
                fft_im[k + j] = ui + ti;
                fft_re[k + j + half] = ur - tr;
                fft_im[k + j + half] = ui - ti;
            }
        }
    }

    // Divide por N una sola vez al final (convención estándar IFFT)
    for (int k = 0; k < PERIOD; k++) {
        resultT[k] = fft_re[k] / PERIOD;
    }
}

// ─── buildFilter ─────────────────────────────────────────────────────────────
/*
 * Construye H[k] como un filtro de banda rectangular con magnitud variable.
 *
 *   Dentro de [binLow .. binHigh]:  H[k] = magnitude
 *   Fuera  de [binLow .. binHigh]:  H[k] = Q15_ONE - magnitude
 *
 * Casos particulares:
 *   Pasa bajo     → binLow=0,    binHigh=fc,   magnitude=Q15_ONE
 *   Pasa alto     → binLow=fc,   binHigh=N/2,  magnitude=Q15_ONE
 *   Pasa banda    → binLow=f1,   binHigh=f2,   magnitude=Q15_ONE
 *   Rechaza banda → binLow=f1,   binHigh=f2,   magnitude=0
 *
 * El espectro se rellena simétricamente (bin k ↔ bin PERIOD-k)
 * para garantizar que la IFFT devuelva una señal real.
 *
 * filterH es int16_t* (antes int32_t*): ahorra la mitad de RAM y permite
 * que applyFilter omita el clamp en su hot path.
 */
void buildFilter(int16_t* filterH, int binLow, int binHigh, int16_t magnitude) {
    if (magnitude < 0)
        magnitude = 0;
    if (magnitude > Q15_ONE)
        magnitude = Q15_ONE;
    int16_t outside = (int16_t)(Q15_ONE - magnitude);
    for (int i = 0; i < PERIOD; i++) {
        int pos = i <= PERIOD / 2 ? i : PERIOD - i;
        filterH[i] = (pos >= binLow && pos <= binHigh) ? magnitude : outside;
    }
}

// ─── applyFilter ─────────────────────────────────────────────────────────────
/*
 * Multiplica el espectro complejo (OmR + j*OmI) por H[k] real, in-place.
 * filterH es int16_t*: el clamp ya no es necesario (buildFilter garantiza
 * que todos los valores estén dentro del rango Q15).
 */
void applyFilter(int32_t* OmR, int32_t* OmI, int16_t* filterH) {
    for (int i = 0; i < PERIOD; i++) {
        OmR[i] = mul_q15(OmR[i], filterH[i]);
        OmI[i] = mul_q15(OmI[i], filterH[i]);
    }
}
