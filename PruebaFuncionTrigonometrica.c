#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define LOG2_PERIOD  10
#define PERIOD       (1 << LOG2_PERIOD)
#define ADC_CENTER   2048
#define ADC_MAX      4095
#define M_PI 3.14159265358979323846
static int16_t tw_cos[PERIOD];
static int16_t tw_sin[PERIOD];
static int64_t fft_re[PERIOD];
static int64_t fft_im[PERIOD];
//no esta en el MCUXPRESSO, asi que la implementamos nosotros
long int lround_custom(double x) {
    if (x >= 0.0) {
        return (long int)(x + 0.5);
    } else {
        return (long int)(x - 0.5);
    }
}

static inline int64_t mul_q15_64(int64_t a, int16_t b) {
    return (a * (int64_t)b + 16384) >> 15;
}

static void bit_reverse64(int64_t* re, int64_t* im) {
    for (int i = 0; i < PERIOD; i++) {
        unsigned int x = (unsigned int)i;
        unsigned int r = 0;
        for (int b = 0; b < LOG2_PERIOD; b++) {
            r = (r << 1) | (x & 1);
            x >>= 1;
        }
        if ((int)r > i) {
            int64_t t;
            t = re[i]; re[i] = re[r]; re[r] = t;
            t = im[i]; im[i] = im[r]; im[r] = t;
        }
    }
}

void init_twiddle(void) {
    for (int k = 0; k < PERIOD; k++) {
        double angle = 2.0 * M_PI * k / PERIOD;
        tw_cos[k] = (int16_t)lround_custom(cos(angle) * 32767.0);
        tw_sin[k] = (int16_t)lround_custom(sin(angle) * 32767.0);
    }
}

void FFT(uint16_t* sourceT, int64_t* sourceR, int64_t* sourceI) {
    for (int i = 0; i < PERIOD; i++) {
        fft_re[i] = (int64_t)sourceT[i] - ADC_CENTER;
        fft_im[i] = 0;
    }
    bit_reverse64(fft_re, fft_im);
    for (int stage = 0; stage < LOG2_PERIOD; stage++) {
        int len  = 1 << (stage + 1);
        int half = len >> 1;
        int step = PERIOD / len;
        for (int k = 0; k < PERIOD; k += len) {
            for (int j = 0; j < half; j++) {
                int     tw_idx = j * step;
                int16_t wr     =  tw_cos[tw_idx & (PERIOD - 1)];
                int16_t wi     = -tw_sin[tw_idx & (PERIOD - 1)];
                int64_t ur = fft_re[k + j];
                int64_t ui = fft_im[k + j];
                int64_t vr = fft_re[k + j + half];
                int64_t vi = fft_im[k + j + half];
                int64_t tr = mul_q15_64(vr, wr) - mul_q15_64(vi, wi);
                int64_t ti = mul_q15_64(vr, wi) + mul_q15_64(vi, wr);
                fft_re[k + j]        = ur + tr;
                fft_im[k + j]        = ui + ti;
                fft_re[k + j + half] = ur - tr;
                fft_im[k + j + half] = ui - ti;
            }
        }
    }
    for (int k = 0; k < PERIOD; k++) {
        sourceR[k] = fft_re[k];
        sourceI[k] = fft_im[k];
    }
}

void IFFT(int64_t* sourceR, int64_t* sourceI, int32_t* resultT) {
    for (int i = 0; i < PERIOD; i++) {
        fft_re[i] = sourceR[i];
        fft_im[i] = sourceI[i];
    }
    bit_reverse64(fft_re, fft_im);
    for (int stage = 0; stage < LOG2_PERIOD; stage++) {
        int len  = 1 << (stage + 1);
        int half = len >> 1;
        int step = PERIOD / len;
        for (int k = 0; k < PERIOD; k += len) {
            for (int j = 0; j < half; j++) {
                int     tw_idx = j * step;
                int16_t wr     =  tw_cos[tw_idx & (PERIOD - 1)];
                int16_t wi     =  tw_sin[tw_idx & (PERIOD - 1)];
                int64_t ur = fft_re[k + j];
                int64_t ui = fft_im[k + j];
                int64_t vr = fft_re[k + j + half];
                int64_t vi = fft_im[k + j + half];
                int64_t tr = mul_q15_64(vr, wr) - mul_q15_64(vi, wi);
                int64_t ti = mul_q15_64(vr, wi) + mul_q15_64(vi, wr);
                fft_re[k + j]        = ur + tr;
                fft_im[k + j]        = ui + ti;
                fft_re[k + j + half] = ur - tr;
                fft_im[k + j + half] = ui - ti;
            }
        }
    }
    for (int k = 0; k < PERIOD; k++) {
        resultT[k] = (int32_t)(fft_re[k] / PERIOD);
    }
}

/* ================================================================
 * compare_signals() — comparador generico entre señal original
 * (centrada, como la ve la FFT) y la señal recuperada por la IFFT.
 *   original_centered: ArrayT[i] - ADC_CENTER  (int32, centrado)
 *   recovered:         salida de IFFT           (int32, centrado)
 * ================================================================ */
void compare_signals(int32_t* original_centered, int32_t* recovered) {
    int64_t max_err = 0, sum_sq = 0;
    int     worst_idx = 0;

    for (int i = 0; i < PERIOD; i++) {
        int64_t err = (int64_t)recovered[i] - original_centered[i];
        if (err < 0) err = -err;
        if (err > max_err) { max_err = err; worst_idx = i; }
        sum_sq += err * err;
    }
    double rms      = sqrt((double)sum_sq / PERIOD);
    double max_amp  = ADC_MAX / 2.0;           /* amplitud pico del ADC */
    double err_pct  = (rms / max_amp) * 100.0; /* error RMS como % del fondo de escala */

    printf("\n--- Comparacion original vs recuperada ---\n");
    printf("Error maximo : %lld  (muestra %d)\n", (long long)max_err, worst_idx);
    printf("Error RMS    : %.3f\n", rms);
    printf("Error RMS    : %.4f %%  del fondo de escala ADC (±%d)\n",
           err_pct, (int)max_amp);
}

/* ================================================================
 * Generadores de señal — todas en rango ADC 0..ADC_MAX
 * ================================================================ */
void setFuncSqr(uint16_t* ArrayT) {
    for (int i = 0; i < PERIOD / 2; i++) ArrayT[i] = 0;
    for (int i = PERIOD / 2; i < PERIOD; i++) ArrayT[i] = ADC_MAX;
}

void setFuncSine(uint16_t* ArrayT) {
    for (int i = 0; i < PERIOD; i++) {
        double v = ADC_CENTER + (ADC_MAX / 2.0) * sin(2.0 * M_PI * i / PERIOD);
        ArrayT[i] = (uint16_t)(v < 0 ? 0 : v > ADC_MAX ? ADC_MAX : v);
    }
}

void setFuncSawtooth(uint16_t* ArrayT) {
    /* sube linealmente de 0 a ADC_MAX */
    for (int i = 0; i < PERIOD; i++)
        ArrayT[i] = (uint16_t)((uint32_t)i * ADC_MAX / (PERIOD - 1));
}

void setFuncTriangle(uint16_t* ArrayT) {
    /* sube hasta el medio, baja hasta el final */
    for (int i = 0; i < PERIOD / 2; i++)
        ArrayT[i] = (uint16_t)((uint32_t)i * ADC_MAX / (PERIOD / 2 - 1));
    for (int i = PERIOD / 2; i < PERIOD; i++)
        ArrayT[i] = (uint16_t)(ADC_MAX - (uint32_t)(i - PERIOD / 2) * ADC_MAX / (PERIOD / 2 - 1));
}

/* ================================================================
 * run_test() — ejecuta FFT→IFFT sobre una señal y muestra todo
 * ================================================================ */
void run_test(const char* name, uint16_t* ArrayT,
              int64_t* OmR, int64_t* OmI, int32_t* Rec) {

    printf("\n");
    printf("========================================\n");
    printf(" Senal: %s\n", name);
    printf("========================================\n");

    /* Señal de entrada centrada (como la procesa la FFT) */
    static int32_t orig_centered[PERIOD];
    for (int i = 0; i < PERIOD; i++)
        orig_centered[i] = (int32_t)ArrayT[i] - ADC_CENTER;

    printf("Input (centrada):\n[");
    for (int i = 0; i < PERIOD; i++) {
        printf("%d", orig_centered[i]);
        if (i < PERIOD - 1) printf(",");
    }
    printf("]\n");

    FFT(ArrayT, OmR, OmI);

    printf("FFT Real:\n[");
    for (int i = 0; i < PERIOD; i++) {
        printf("%lld", (long long)OmR[i]);
        if (i < PERIOD - 1) printf(",");
    }
    printf("]\n");

    printf("FFT Imag:\n[");
    for (int i = 0; i < PERIOD; i++) {
        printf("%lld", (long long)OmI[i]);
        if (i < PERIOD - 1) printf(",");
    }
    printf("]\n");

    IFFT(OmR, OmI, Rec);

    printf("Recovered:\n[");
    for (int i = 0; i < PERIOD; i++) {
        printf("%d", Rec[i]);
        if (i < PERIOD - 1) printf(",");
    }
    printf("]\n");

    compare_signals(orig_centered, Rec);
}

/* ================================================================
 * main
 * ================================================================ */
int main(void) {
    static uint16_t ArrayT[PERIOD];
    static int64_t  OmR[PERIOD], OmI[PERIOD];
    static int32_t  Rec[PERIOD];

    init_twiddle();

    printf("=== FFT/IFFT test  N=%d  ADC_CENTER=%d ===", PERIOD, ADC_CENTER);

    setFuncSqr(ArrayT);      run_test("Cuadrada",  ArrayT, OmR, OmI, Rec);
    setFuncSine(ArrayT);     run_test("Seno",       ArrayT, OmR, OmI, Rec);
    setFuncSawtooth(ArrayT); run_test("Sawtooth",   ArrayT, OmR, OmI, Rec);
    setFuncTriangle(ArrayT); run_test("Triangular", ArrayT, OmR, OmI, Rec);

    return 0;
}
