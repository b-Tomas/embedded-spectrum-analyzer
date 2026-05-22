#include "lpc17xx.h"
#include <stdint.h>

/* ================================================================
 * Configuracion — solo tocar LOG2_PERIOD
 * Soporta: 5=32, 6=64, 7=128, 8=256, 9=512, 10=1024,
 *          11=2048, 12=4096, 13=8192, 14=16384, 15=32768
 *          por el teorema del muestreo 16kHz son perfectos para el oido humano
 * ================================================================ */
#define LOG2_PERIOD   7
#define PERIOD        (1 << LOG2_PERIOD)

/* ================================================================
 * NOTA: ADC del LPC17xx — rango 0..2047, centro = 1024
 * La FFT centra restando 1024 antes de procesar funciona como la matematica formal.
 * ================================================================ */
#define ADC_CENTER    1024

/* ================================================================
 * Tablas de twiddle — generadas una vez por init_twiddle()
 * Colocar en la RAM
 * ================================================================ */
static int16_t tw_cos[PERIOD];
static int16_t tw_sin[PERIOD];

/* Semillas: cos(2*pi / 2^k) * 32767  para k = 0..15 */
static const int32_t seed_cos[16] = {
     32767,  /* k= 0 */
    -32767,  /* k= 1 */
         0,  /* k= 2 */
     23170,  /* k= 3 */
     30273,  /* k= 4 */
     32138,  /* k= 5 */
     32610,  /* k= 6 */
     32729,  /* k= 7 */
     32758,  /* k= 8 */
     32766,  /* k= 9 */
     32767,  /* k=10 */
     32767,  /* k=11 */
     32767,  /* k=12 */
     32767,  /* k=13 */
     32767,  /* k=14 */
     32767,  /* k=15 */
};

static const int32_t seed_sin[16] = {
         0,  /* k= 0 */
         0,  /* k= 1 */
     32767,  /* k= 2 */
     23170,  /* k= 3 */
     12539,  /* k= 4 */
      6393,  /* k= 5 */
      3212,  /* k= 6 */
      1608,  /* k= 7 */
       804,  /* k= 8 */
       402,  /* k= 9 */
       201,  /* k=10 */
       100,  /* k=11 */
        50,  /* k=12 */
        25,  /* k=13 */
        13,  /* k=14 */
         6,  /* k=15 */
};

/* ================================================================
 * Buffers internos de trabajo de la FFT
 * ================================================================ */
static int32_t fft_re[PERIOD];
static int32_t fft_im[PERIOD];

/* ================================================================
 * Funciones internas
 * ================================================================ */
static inline int32_t mul_q15(int32_t a, int16_t b) {
    return (a * (int32_t)b) >> 15;
}

static void bit_reverse(int32_t* re, int32_t* im) {
    for (int i = 0; i < PERIOD; i++) {
        unsigned int x = (unsigned int)i;
        unsigned int r = 0;
        for (int b = 0; b < LOG2_PERIOD; b++) {
            r = (r << 1) | (x & 1);
            x >>= 1;
        }
        if ((int)r > i) {
            int32_t t;
            t = re[i]; re[i] = re[r]; re[r] = t;
            t = im[i]; im[i] = im[r]; im[r] = t;
        }
    }
}

/* ================================================================
 * init_twiddle()
 * Genera las tablas tw_cos[] / tw_sin[] para PERIOD puntos.
 * Llamar UNA SOLA VEZ al inicio, antes de cualquier llamada a FFT().
 * ================================================================ */
void init_twiddle(void) {
    int32_t cd = seed_cos[LOG2_PERIOD];
    int32_t sd = seed_sin[LOG2_PERIOD];
    int32_t cr = 32767;
    int32_t sr = 0;

    for (int k = 0; k < PERIOD; k++) {
        tw_cos[k] = (int16_t)cr;
        tw_sin[k] = (int16_t)sr;

        int32_t cr_new = ((cr * cd) >> 15) - ((sr * sd) >> 15);
        int32_t sr_new = ((sr * cd) >> 15) + ((cr * sd) >> 15);
        cr = cr_new;
        sr = sr_new;
    }
}

/* ================================================================
 * FFT()
 *
 * Parametros:
 *   sourceT  — array de entrada uint16_t[PERIOD] con muestras del ADC
 *              rango 0..2047, centro en 1024
 *   sourceR  — array de salida int32_t[PERIOD] parte Real
 *   sourceI  — array de salida int32_t[PERIOD] parte Imaginaria
 *
 * Al retornar, sourceR[k] y sourceI[k] contienen el bin k de la FFT.
 *
 *
 * Precondicion: init_twiddle() debe haberse llamado antes.
 * ================================================================ */
void FFT(uint16_t* sourceT, int32_t* sourceR, int32_t* sourceI) {
    /* Cargar entrada centrada en cero */
    for (int i = 0; i < PERIOD; i++) {
        fft_re[i] = (int32_t)sourceT[i] - ADC_CENTER;
        fft_im[i] = 0;
    }

    bit_reverse(fft_re, fft_im);

    for (int stage = 0; stage < LOG2_PERIOD; stage++) {
        int len  = 1 << (stage + 1);
        int half = len >> 1;
        int step = PERIOD / len;

        for (int k = 0; k < PERIOD; k += len) {
            for (int j = 0; j < half; j++) {
                int     tw_idx = j * step;
                int16_t wr     =  tw_cos[tw_idx & (PERIOD - 1)];
                int16_t wi     = -tw_sin[tw_idx & (PERIOD - 1)];

                int32_t ur = fft_re[k + j];
                int32_t ui = fft_im[k + j];
                int32_t vr = fft_re[k + j + half];
                int32_t vi = fft_im[k + j + half];

                int32_t tr = mul_q15(vr, wr) - mul_q15(vi, wi);
                int32_t ti = mul_q15(vr, wi) + mul_q15(vi, wr);

                fft_re[k + j]        = (ur + tr) >> 1;
                fft_im[k + j]        = (ui + ti) >> 1;
                fft_re[k + j + half] = (ur - tr) >> 1;
                fft_im[k + j + half] = (ui - ti) >> 1;
            }
        }
    }

    for (int k = 0; k < PERIOD; k++) {
        sourceR[k] = fft_re[k];
        sourceI[k] = fft_im[k];
    }
}
int main(void){
while(1){
__WFI();
}

}
