#include "LPC17xx.h"
#include "display/display.h"
#include "display/i2c_bus.h"
#include "gpdma/gpdma.h"
#include "input/keyboard.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_timer.h"
#include "lpc_types.h"
#include "system/eq_mode.h"
#include "system/system.h"

#include <stdbool.h>
#include <stdio.h>

// Uncomment the line below to run all integration tests
// TODO(b-Tomas): find a cleaner way to run tests
// #define RUN_TESTS

int main(void) {
#ifdef RUN_TESTS
#include "test/it.h"
    it_run_all();
#endif

    system_Init(realTimeMode);

    // ReSharper disable once CppDFAEndlessLoop
    while (true) {
        __WFI();
        // Process keyboard input if any
        char c;
        while (kbd_pop(&c)) {
            printf("pressed key %c\n", c);
            if (c == 'A' || c == 'B' || c == 'C') {
                // Change mode if applicable
                SYSTEM.flag_ModeConfigured = RESET;
                switch (c) {
                case 'A':
                    system_setMode(realTimeMode);
                    break;
                case 'B':
                    system_setMode(noiseSamplingMode);
                    break;
                case 'C':
                    system_setMode(equalizerMode);
                    break;
                default:
                    break;
                }
                // HACK: clear the display
                display_clearCanvas();
                display_displayCanvas();
            } else {
                // The current mode consumes the key
                switch (SYSTEM.mode) {
                case equalizerMode:
                    system_processKeyEqualizerMode(c);
                    break;
                default:
                    break;
                }
            }
        }

        switch (SYSTEM.mode) {
        case realTimeMode:
            if (!SYSTEM.flag_ModeConfigured) {
                system_ConfigureSetting_RealTimeMode();
                SYSTEM.flag_ModeConfigured = SET;
            }
            system_tickRealTimeMode();
            break;

        case noiseSamplingMode:
            if (!SYSTEM.flag_ModeConfigured) {
                SYSTEM.flag_ModeConfigured = SET;
                system_ConfigureSetting_NoiseSamplingMode();
            }
            system_tickNoiseSamplingMode();
            break;

        case equalizerMode:
            if (!SYSTEM.flag_ModeConfigured) {
                SYSTEM.flag_ModeConfigured = SET;
                system_ConfigureSetting_EqualizerMode();
            }
            system_tickEqualizerMode();
            break;
        }
    }
    return 0;
}

void EINT0_IRQHandler() {
    kbd_irq_handler();
    EXTI_ClearFlag(EXTI_EINT0);
}

void I2C0_IRQHandler() {
    i2c_irq_handler();
}


void TIMER0_IRQHandler(void) {
    flag_SamplingCooldownReady = SET; /* ← era flag_readyToDisplay */
    TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
}

void TIMER1_IRQHandler(void) {
    TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
    flag_readyToDisplay = SET;
}

void DMA_IRQHandler(void) {
    gpdma_irq_handler();
}