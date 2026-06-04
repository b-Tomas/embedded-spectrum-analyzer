#include "lpc17xx.h"
#include "lpc17xx_timer.h"
#include "fft.h"
#include <stdio.h>

#define AHB_RAM __attribute__((section(".data_RAM2")))

static AHB_RAM uint16_t signal_in [PERIOD];
static AHB_RAM int32_t  freqR     [PERIOD];
static AHB_RAM int32_t  freqI     [PERIOD];
static AHB_RAM int32_t  filterH   [PERIOD];
static AHB_RAM int32_t  output    [PERIOD];

int main(void) {
    TIM_TIMERCFG_T cfg = { TIM_US, 1 };
    TIM_InitTimer(LPC_TIM0, &cfg);
    TIM_Enable(LPC_TIM0);

    // señal de prueba
    for (int i = 0; i < PERIOD; i++)
        signal_in[i] = (i < PERIOD / 2) ? 3000 : 1000;

    // --- FFT ---
    TIM_ResetCounter(LPC_TIM0);
    FFT(signal_in, freqR, freqI);
    uint32_t t_fft = TIM_ReadTimer(LPC_TIM0);

    // --- IFFT con filtro ---
    buildFilter(filterH, 0, 50, Q15_ONE);
    TIM_ResetCounter(LPC_TIM0);
    applyFilter(freqR, freqI, filterH);
    IFFT(freqR, freqI, output);
    uint32_t t_ifft = TIM_ReadTimer(LPC_TIM0);

    printf("FFT:              %lu us\r\n", t_fft);
    printf("IFFT con filtro:  %lu us\r\n", t_ifft);

    while (1) {}
}
