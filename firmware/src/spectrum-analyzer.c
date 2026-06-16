#include "LPC17xx.h"
#include "display/i2c_bus.h"
#include "gpdma/gpdma.h"
#include "input/keyboard.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_timer.h"
#include "lpc_types.h"
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
    system_init(realTimeMode);
    // ReSharper disable once CppDFAEndlessLoop
    while (true) {
        __WFI();
        // Process keyboard input if any
        char c;
        while (kbd_pop(&c)) {
            printf("pressed key %c\n", c);
            if (c == 'A' || c == 'B' || c == 'C') {
                // Change mode if applicable
                MODES[SYSTEM.mode].deInit();
                system_setMode(c == 'A'   ? realTimeMode
                               : c == 'B' ? noiseSamplingMode
                                          : equalizerMode);
                SYSTEM.flag_ModeConfigured = RESET;
            } else {
                // The current mode consumes the key
                MODES[SYSTEM.mode].handleKey(c);
            }
        }

        if (!SYSTEM.flag_ModeConfigured) {
            MODES[SYSTEM.mode].init();
            SYSTEM.flag_ModeConfigured = SET;
        }
        MODES[SYSTEM.mode].tick();
    }
}

void EINT0_IRQHandler() {
    kbd_irq_handler();
    EXTI_ClearFlag(EXTI_EINT0);
}

void I2C0_IRQHandler() {
    i2c_irq_handler();
}

void TIMER1_IRQHandler(void) {
    TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
    SYSTEM.flag_readyToDisplay = SET;
}

void DMA_IRQHandler(void) {
    gpdma_irq_handler();
}
