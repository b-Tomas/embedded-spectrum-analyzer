#include "system/system.h"

#include "adc/adc.h"
#include "display/display.h"
#include "gpdma/gpdma.h"
#include "input/keyboard.h"
#include "lpc17xx_dac.h"
#include "lpc_types.h"
#include "system/eq_mode.h"
#include "system/noise_sampling_mode.h"
#include "system/real_time_mode.h"

System_T SYSTEM;

SystemMode_T MODES[_modeCount];

void system_init(Mode const mode) {
    // Register all mode hooks
    eqMode_registerHooks();
    noiseSamplingMode_registerHooks();
    realTimeMode_registerHooks();
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
}

void system_setMode(Mode const mode) {
    SYSTEM.mode = mode;
    SYSTEM.flag_ModeConfigured = RESET;
}

void system_registerModeHooks(Mode const mode, SystemMode_T const* hooks) {
    MODES[mode] = *hooks;
}
