#include "system/system.h"

#include "adc/adc.h"
#include "display/display.h"
#include "gpdma/gpdma.h"
#include "input/keyboard.h"
#include "lpc17xx_dac.h"
#include "lpc_types.h"

System_T SYSTEM;

static SystemMode_T MODES[MODE_COUNT];

void system_init(Mode const mode) {
    // Register all mode hooks
    realTimeMode_registerHooks();
    noiseSamplingMode_registerHooks();
    eqMode_registerHooks();
    // Initialize each subsystem
    // some may start async processes that produce interrupt. The ordering is important to prevent
    // deadlocks
    display_init();
    adc_init();
    gpdma_init();
    DAC_Init();
    kbd_init();
    // Initialize the given mode
    system_setMode(mode);

    // Reset the frequency-domain filter to passthrough
    for (int i = 0; i < PERIOD; i++) {
        FILTER_H[i] = Q15_ONE;
    }
}

void system_setMode(Mode const mode) {
    SYSTEM.mode = mode;
    SYSTEM.flag_ModeConfigured = RESET;
}

void system_registerMode(Mode const mode, SystemMode_T const* hooks) {
    MODES[mode] = *hooks;
}

void system_tick(void) {
    if (!SYSTEM.flag_ModeConfigured) {
        MODES[SYSTEM.mode].init();
        SYSTEM.flag_ModeConfigured = SET;
    }
    MODES[SYSTEM.mode].tick();
}

void system_handleKey(char const c) {
    if (c == 'A' || c == 'B' || c == 'C') {
        MODES[SYSTEM.mode].deInit();
        system_setMode(c == 'A' ? realTimeMode : c == 'B' ? noiseSamplingMode : equalizerMode);
    } else {
        MODES[SYSTEM.mode].handleKey(c);
    }
}
