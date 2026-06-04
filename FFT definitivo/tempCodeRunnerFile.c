#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "fft.h"
#include <math.h>

static uint16_t  ArrayT[PERIOD];
static int32_t   OmR[PERIOD], OmI[PERIOD];       // ← int32_t
static int32_t   OmR_bak[PERIOD], OmI_bak[PERIOD]; // ← int32_t
static int32_t   Rec[PERIOD];
static int32_t   filterH[PERIOD];                 // ← int32_t

typedef enum {
    WAVE_SQUARE,
    WAVE_SINE,
    WAVE_TRIANGLE,
    WAVE_SAWTOOTH
} WaveType;

void setFunc(uint16_t* arr, WaveType wave) {
    for (int i = 0; i < PERIOD; i++) {
        switch (wave) {
            case WAVE_SQUARE:
                arr[i] = (i < PERIOD / 2) ? ADC_MAX : 0;
                break;
            case WAVE_SINE:
                arr[i] = (uint16_t)((sin(2 * 3.14159 * i / PERIOD) + 1) * ADC_CENTER);
                break;
            case WAVE_TRIANGLE:
                if (i < PERIOD / 2)
                    arr[i] = (uint16_t)(i * ADC_MAX / (PERIOD / 2));
                else
                    arr[i] = (uint16_t)((PERIOD - i) * ADC_MAX / (PERIOD / 2));
                break;
            case WAVE_SAWTOOTH:
                arr[i] = (uint16_t)(i * ADC_MAX / (PERIOD - 1));
                break;
        }
    }
}

int main(void) {

    setFunc(ArrayT, WAVE_SQUARE);

    printf("[");
    for (int i = 0; i < PERIOD; i++) {
        printf("%d", ArrayT[i] - ADC_CENTER);
        if (i < PERIOD - 1) printf(",");
    }
    printf("]\n\n\n");

    FFT(ArrayT, OmR, OmI);
    memcpy(OmR_bak, OmR, sizeof(OmR));
    memcpy(OmI_bak, OmI, sizeof(OmI));

    buildFilter(filterH, 0, 5, Q15_ONE);

    applyFilter(OmR, OmI, filterH);
    IFFT(OmR, OmI, Rec);

    printf("[");
    for (int i = 0; i < PERIOD; i++) {
        printf("%d", Rec[i]);
        if (i < PERIOD - 1) printf(",");
    }
    printf("]\n");

    return 0;
}