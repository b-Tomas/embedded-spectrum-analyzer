#include <stdio.h>
#include <stdint.h>
#include <math.h>
//gcc main.c fft.c -o main.exe ; .\main.exe
//ejecutar esto en la terminal del vs code para generar las tablas y luego copiar/pegar el resultado en fft.c
#define LOG2_PERIOD  10
#define PERIOD       (1 << LOG2_PERIOD)
#define PI           3.14159265358979323846
#define VALUES_PER_LINE 12

// ─── Tablas a generar ────────────────────────────────────────────────────────
static int16_t  tw_cos[PERIOD];
static uint16_t bit_rev_table[PERIOD];

// ─── Redondeo sin depender de lround() ───────────────────────────────────────
static long int round_nearest(double x) {
    return (x >= 0.0) ? (long int)(x + 0.5) : (long int)(x - 0.5);
}

// ─── Genera tw_cos (sin tw_sin: sin(k) = cos(k + 3·N/4)) ────────────────────
static void init_twiddle(void) {
    for (int k = 0; k < PERIOD; k++) {
        double angle = 2.0 * PI * k / PERIOD;
        tw_cos[k] = (int16_t)round_nearest(cos(angle) * 32767.0);
    }
}

// ─── Genera bit_rev_table ────────────────────────────────────────────────────
static uint16_t bit_reverse_index(uint16_t x) {
    uint16_t r = 0;
    for (int b = 0; b < LOG2_PERIOD; b++) {
        r = (uint16_t)((r << 1) | (x & 1));
        x >>= 1;
    }
    return r;
}

static void init_bit_rev_table(void) {
    for (int i = 0; i < PERIOD; i++) {
        bit_rev_table[i] = bit_reverse_index((uint16_t)i);
    }
}

// ─── Impresión formateada lista para pegar en fft.c ──────────────────────────
static void print_cos_table(void) {
    printf("static const int16_t tw_cos[%d] = {\n", PERIOD);
    for (int i = 0; i < PERIOD; i++) {
        if (i % VALUES_PER_LINE == 0) printf("    ");
        printf("%d", tw_cos[i]);
        if (i < PERIOD - 1) {
            printf(",");
            if ((i + 1) % VALUES_PER_LINE == 0) printf("\n");
        }
    }
    printf("\n};\n");
}

static void print_bit_rev_table(void) {
    printf("static const uint16_t bit_rev_table[%d] = {\n", PERIOD);
    for (int i = 0; i < PERIOD; i++) {
        if (i % VALUES_PER_LINE == 0) printf("    ");
        printf("%u", bit_rev_table[i]);
        if (i < PERIOD - 1) {
            printf(",");
            if ((i + 1) % VALUES_PER_LINE == 0) printf("\n");
        }
    }
    printf("\n};\n");
}

int main(void) {
    init_twiddle();
    init_bit_rev_table();

    printf("/* Generado para LOG2_PERIOD=%d, PERIOD=%d */\n\n", LOG2_PERIOD, PERIOD);

    print_cos_table();
    printf("\n");
    print_bit_rev_table();

    return 0;
}