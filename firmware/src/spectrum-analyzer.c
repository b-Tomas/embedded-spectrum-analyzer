
#include "LPC17xx.h"
#include "lpc_types.h"
#include "system.h"

#include <stdbool.h>

int main(void) {

    systemInit(realTimeMode);

    while (TRUE) {

        __WFI();
        switch (SYSTEM.mode) {
        case realTimeMode:
            if (!SYSTEM.flag_ModeConfigured) {
                configRealTimeMode();
                SYSTEM.flag_ModeConfigured = SET;
            }

            executeRealTimeMode();

            break;

        case noiseSamplingMode:
            if (!SYSTEM.flag_ModeConfigured) {
                SYSTEM.flag_ModeConfigured = SET;
                configNoiseSamplingMode();
                executeNoiseSamplingMode();
            }

            break;

        case equalizerMode:
            if (!SYSTEM.flag_ModeConfigured) {
                SYSTEM.flag_ModeConfigured = SET;
                configEqualizerMode();
                executeEqualizerMode();
            }

            break;
        }
    }

    return 0;
}
