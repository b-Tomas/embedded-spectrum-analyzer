#include <stdio.h>
#include <stdint.h>

/* ================================================================
 * Configuracion — solo tocar LOG2_PERIOD
 * Soporta: 5=32, 6=64, 7=128, 8=256, 9=512, 10=1024,
 *          11=2048, 12=4096, 13=8192, 14=16384, 15=32768
 * ================================================================ */
#define LOG2_PERIOD   6
#define PERIOD        (1 << LOG2_PERIOD)

#define AXIS_WIDTH    200
#define SCALAR        20

/* ================================================================
 * Memoria — reemplazar por SRAM del micro si es necesario
 * Nota: 32k muestras * 2 bytes = 64 KB para ArrayT
 *       32k muestras * 4 bytes = 128 KB por cada ArrayFR / ArrayFI
 * ================================================================ */
static uint16_t memory1[PERIOD];
static int32_t  memory2[PERIOD];
static int32_t  memory3[PERIOD];

uint16_t* ArrayT  = memory1;
int32_t*  ArrayFR = memory2;
int32_t*  ArrayFI = memory3;

/* ================================================================
 * Tablas de twiddle generadas en tiempo de ejecucion
 * Se calculan una sola vez en init_twiddle()
 * ================================================================ */
static int16_t tw_cos[PERIOD];
static int16_t tw_sin[PERIOD];

/*
 * Semillas: cos(2*pi / 2^k) * 32767  y  sin(2*pi / 2^k) * 32767
 * para k = 0 .. 15
 *
 * Calculadas con: round(cos/sin(2*pi / 2^k) * 32767)
 *
 * k  | 2^k    | cos seed  | sin seed
 * ---|--------|-----------|----------
 *  0 |      1 |     32767 |        0   (cos(2pi)=1, sin(2pi)=0)
 *  1 |      2 |    -32767 |        0   (cos(pi)=-1, sin(pi)=0)
 *  2 |      4 |         0 |    32767   (cos(pi/2)=0, sin(pi/2)=1)
 *  3 |      8 |     23170 |    23170   (cos(pi/4)=sin(pi/4)=√2/2)
 *  4 |     16 |     30273 |    12539
 *  5 |     32 |     32138 |     6393
 *  6 |     64 |     32610 |     3212
 *  7 |    128 |     32729 |     1608
 *  8 |    256 |     32758 |      804
 *  9 |    512 |     32766 |      402
 * 10 |   1024 |     32767 |      201
 * 11 |   2048 |     32767 |      100
 * 12 |   4096 |     32767 |       50
 * 13 |   8192 |     32767 |       25
 * 14 |  16384 |     32767 |       13
 * 15 |  32768 |     32767 |        6
 */
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

/*
 * Genera las tablas tw_cos[] y tw_sin[] para PERIOD puntos.
 *
 * Metodo: recurrencia de rotacion compleja en Q15
 *   cos(a + d) = cos(a)*cos(d) - sin(a)*sin(d)
 *   sin(a + d) = sin(a)*cos(d) + cos(a)*sin(d)
 * donde d = 2*pi/PERIOD (paso angular fijo).
 *
 * Acumulacion de error: para PERIOD=32768 el error acumulado
 * es de +-2 LSB en Q15, aceptable para FFT de audio/instrumentacion.
 */
void init_twiddle(void) {
    int32_t cd = seed_cos[LOG2_PERIOD];  /* cos del paso angular en Q15 */
    int32_t sd = seed_sin[LOG2_PERIOD];  /* sin del paso angular en Q15 */

    int32_t cr = 32767;  /* cos(0) = 1 en Q15 */
    int32_t sr = 0;      /* sin(0) = 0        */

    for (int k = 0; k < PERIOD; k++) {
        tw_cos[k] = (int16_t)cr;
        tw_sin[k] = (int16_t)sr;

        /* Rotar (cr, sr) por (cd, sd) */
        int32_t cr_new = ((cr * cd) >> 15) - ((sr * sd) >> 15);
        int32_t sr_new = ((sr * cd) >> 15) + ((cr * sd) >> 15);
        cr = cr_new;
        sr = sr_new;
    }
}

static inline int16_t tw_get_cos(int k) { return tw_cos[k & (PERIOD - 1)]; }
static inline int16_t tw_get_sin(int k) { return tw_sin[k & (PERIOD - 1)]; }

/* ================================================================
 * Aritmetica Q15
 * ================================================================ */
static inline int32_t mul_q15(int32_t a, int16_t b) {
    return (a * (int32_t)b) >> 15;
}

/* ================================================================
 * Bit-reversal generico para cualquier potencia de 2
 * ================================================================ */
static void bit_reverse(int32_t* re, int32_t* im) {
    for (int i = 0; i < PERIOD; i++) {
        /* Revertir LOG2_PERIOD bits de i */
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
 * FFT en punto fijo, sin float
 * ================================================================ */
static int32_t fft_re[PERIOD];
static int32_t fft_im[PERIOD];

void FFT(uint16_t* sourceT, int32_t* sourceR, int32_t* sourceI) {
    /* Cargar entrada centrada en cero */
    for (int i = 0; i < PERIOD; i++) {
        fft_re[i] = (int32_t)sourceT[i] - 2048;
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
                int16_t wr     =  tw_get_cos(tw_idx);
                int16_t wi     = -tw_get_sin(tw_idx);

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

/* ================================================================
 * Visualizacion en consola
 * ================================================================ */
void printWave(int32_t* source) {
    for (int i = 0; i < PERIOD; i++) {
        int val = source[i];
        if (val >  AXIS_WIDTH) val =  AXIS_WIDTH;
        if (val < -AXIS_WIDTH) val = -AXIS_WIDTH;

        char line[2 * AXIS_WIDTH + 4];
        int pos = 0;
        if (val >= 0) {
            for (int j = 0; j < AXIS_WIDTH; j++) line[pos++] = ' ';
            line[pos++] = '|';
            for (int j = 0; j < val; j++)        line[pos++] = '-';
            line[pos++] = '*';
        } else {
            int bars   = -val;
            int spaces = AXIS_WIDTH - bars;
            for (int j = 0; j < spaces; j++) line[pos++] = ' ';
            line[pos++] = '*';
            for (int j = 0; j < bars; j++)   line[pos++] = '-';
            line[pos++] = '|';
        }
        line[pos] = '\0';
        puts(line);
        fflush(stdout);
    }
}

void printWaveU16_centered(uint16_t* source) {
    int32_t centered[PERIOD];
    for (int i = 0; i < PERIOD; i++)
        centered[i] = (int32_t)source[i] - 2048;
    printWave(centered);
}

/* ================================================================
 * Visualizacion numerica — tabla de valores
 * ================================================================ */

/* Imprime un array int32 como tabla numerica.
 * Parametros:
 *   source   — array de PERIOD valores
 *   label    — nombre de la columna (ej: "Real", "Imag", "Muestra")
 *   fs_hz    — frecuencia de muestreo en Hz (0 = no muestra columna Hz)
 * Formato:
 *   Indice |   Hz   |  Valor
 * Solo imprime los primeros half_only bins si half_only != 0
 * (util para FFT donde la segunda mitad es espejo)
 */
void printTable(int32_t* source, const char* label, int fs_hz, int half_only) {
    int count = half_only ? PERIOD / 2 : PERIOD;

    if (fs_hz > 0)
        printf("%6s | %10s | %s\n", "Bin", "Hz", label);
    else
        printf("%6s | %s\n", "Indice", label);

    printf("-------+------------+------------\n");

    for (int i = 0; i < count; i++) {
        if (fs_hz > 0) {
            /* frecuencia del bin = i * Fs / PERIOD */
            int32_t freq_mhz = ((int32_t)i * fs_hz) / PERIOD; /* en Hz entero */
            printf("%6d | %10d | %ld\n", i, freq_mhz, (long)source[i]);
        } else {
            printf("%6d | %ld\n", i, (long)source[i]);
        }
        fflush(stdout);
    }
}

/* Version para uint16 centrada en 2048.
 * Columnas:
 *   Indice   — numero de muestra
 *   ADC      — valor crudo del ADC (0..4095, centro = 2048)
 *   Centrado — valor centrado en cero (ADC - 2048)
 */
void printTableU16_centered(uint16_t* source) {
    printf("%6s | %6s | %s\n", "Indice", "ADC", "Centrado");
    printf("-------+--------+----------\n");
    for (int i = 0; i < PERIOD; i++) {
        uint16_t adc     = source[i];
        int32_t  centrado = (int32_t)adc - 2048;
        printf("%6d | %6u | %d\n", i, adc, centrado);
        fflush(stdout);
    }
}

/* Imprime Real e Imaginario juntos en una sola tabla.
 * Si fs_hz > 0 agrega columna de frecuencia.
 * Si half_only != 0 solo imprime PERIOD/2 bins (espectro util).
 */
void printTableFFT(int32_t* re, int32_t* im, int fs_hz, int half_only) {
    int count = half_only ? PERIOD / 2 : PERIOD;

    if (fs_hz > 0)
        printf("%6s | %10s | %12s | %12s\n", "Bin", "Hz", "Real", "Imag");
    else
        printf("%6s | %12s | %12s\n", "Bin", "Real", "Imag");

    printf("-------+------------+--------------+--------------\n");

    for (int i = 0; i < count; i++) {
        if (fs_hz > 0) {
            int32_t freq = ((int32_t)i * fs_hz) / PERIOD;
            printf("%6d | %10d | %12ld | %12ld\n",
                   i, freq, (long)re[i], (long)im[i]);
        } else {
            printf("%6d | %12ld | %12ld\n",
                   i, (long)re[i], (long)im[i]);
        }
        fflush(stdout);
    }
}

/* ================================================================
 * Generadores de senales — sin float/double
 * ================================================================ */
void setFunc_Seno(void) {
    for (int i = 0; i < PERIOD; i++) {
        /* Interpola tw_sin que tiene PERIOD muestras exactas */
        int32_t s = ((int32_t)SCALAR * (int32_t)tw_sin[i]) >> 15;
        ArrayT[i] = (uint16_t)(2048 + s);
    }
}

void setFunc_Cuadrada(void) {
    for (int i = 0; i < PERIOD; i++)
        ArrayT[i] = (i < PERIOD / 2) ? 4000 : 96;
}

void setFunc_Triangular(void) {
    for (int i = 0; i < PERIOD; i++) {
        int32_t val;
        if (i < PERIOD / 4)
            val = (4 * SCALAR * i) / PERIOD;
        else if (i < 3 * PERIOD / 4)
            val = SCALAR - (4 * SCALAR * (i - PERIOD / 4)) / PERIOD;
        else
            val = -SCALAR + (4 * SCALAR * (i - 3 * PERIOD / 4)) / PERIOD;
        ArrayT[i] = (uint16_t)(2048 + val);
    }
}

void setFunc_Sawtooth(void) {
    for (int i = 0; i < PERIOD; i++) {
        int32_t val = (2 * SCALAR * i) / PERIOD - SCALAR;
        ArrayT[i] = (uint16_t)(2048 + val);
    }
}

/* ================================================================
 * Main
 * ================================================================ */

/* Elegir modo de visualizacion:
 *   0 = barras ASCII  (printWave / printWaveU16_centered)
 *   1 = tabla numerica (printTable / printTableFFT)
 */
#define PRINT_MODE   0

/* Frecuencia de muestreo en Hz (usado solo en modo tabla para columna Hz).
 * Poner 0 para no mostrar columna de frecuencia.
 */
#define FS_HZ        0

int main(void) {
    /* PASO 1: inicializar tablas de twiddle para el PERIOD elegido */
    init_twiddle();

    setFunc_Seno();

    printf("--- Senal temporal (centrada en 2048) ---\n");
#if PRINT_MODE == 0
    printWaveU16_centered(ArrayT);
#else
    printTableU16_centered(ArrayT);
#endif

    printf("\n|----- FFT (punto fijo, sin float) -----|\n\n");
    fflush(stdout);

    FFT(ArrayT, ArrayFR, ArrayFI);

#if PRINT_MODE == 0
    printf("Parte Real\n");
    printWave(ArrayFR);
    printf("Parte Imag\n");
    printWave(ArrayFI);
#else
    /* Tabla combinada Real+Imag, solo primeros PERIOD/2 bins utiles,
     * con columna de frecuencia en Hz */
    printf("Espectro FFT (primeros %d bins utiles)\n", PERIOD / 2);
    printTableFFT(ArrayFR, ArrayFI, FS_HZ, 1);
#endif

    return 0;
}