
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
            if (!SYSTEM.flag_ModeExecuted) {
                configRealTimeMode();
                SYSTEM.flag_ModeExecuted = SET;
            }

            // TODO: State implementation.
            break;

        case noiseSamplingMode:
            if (!SYSTEM.flag_ModeExecuted) {
                configNoiseSamplingMode();
                SYSTEM.flag_ModeExecuted = SET;
            }

            // TODO: State implementation.
            break;

        case equalizerMode:
            if (!SYSTEM.flag_ModeExecuted) {
                configEqualizerMode();
                SYSTEM.flag_ModeExecuted = SET;
            }

            // TODO: State implementation.
            break;
        }
    }

    return 0;
}
