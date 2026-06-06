#include "LPC17xx.h"
#include "display/i2c_bus.h"
#include "input/keyboard.h"
#include "lpc17xx_exti.h"
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

    system_Init(realTimeMode);
    system_nonConfigurableSetting();

    // ReSharper disable once CppDFAEndlessLoop
    while (true) {
        __WFI();

        // Process keyboard input if any
        char c;
        while (kbd_pop(&c)) {
            // TODO: process the keypress depending on the mode
            printf("Key %c\n", c);
            // switch (SYSTEM.mode) {
            // case realTimeMode:
            //   processKeyRealTimeMode();
            // ...etc
        }

        switch (SYSTEM.mode) {
        case realTimeMode:
            if (!SYSTEM.flag_ModeConfigured) {
                system_ConfigureSetting_RealTimeMode();
                SYSTEM.flag_ModeConfigured = SET;
            }

            system_StartRealTimeMode();

            break;

        case noiseSamplingMode:
            if (!SYSTEM.flag_ModeConfigured) {
                SYSTEM.flag_ModeConfigured = SET;
                system_ConfigureSetting_NoiseSamplingMode();
                system_StartNoiseSamplingMode();
            }
  
            break;

        case equalizerMode:
            if (!SYSTEM.flag_ModeConfigured) {
                SYSTEM.flag_ModeConfigured = SET;
                system_ConfigureSetting_EqualizerMode();
                system_StartEqualizerMode();
            }

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
