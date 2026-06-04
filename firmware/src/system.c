#include "system.h"

#include "display/display.h"
#include "input/keyboard.h"
#include "lpc_types.h"

/** @brief Global orchestrator instance. */
System SYSTEM;

void systemInit(Mode const mode) {
    setMode(mode);
    kbd_init();
    display_init();
    display_clearCanvas();
    display_displayCanvas();
}

void configRealTimeMode(void) {
    // TODO: ADC re-config, GPDMA re-config, DISPLAY re-config
}

void configNoiseSamplingMode(void) {
    // TODO: ADC reconfig, GPDMA re-config, DISPLAY re-config?
}

void configEqualizerMode(void) {
    // TODO: KEYBOARD re-config, ADC-stop? GPDMA-reconfig (select filer), DISPLAY re-config
}

void executeRealTimeMode(void) {
    // TODO:Real time mode implementation.
}

void executeNoiseSamplingMode(void) {
    // TODO: Noise-Sampling mode implementation
}

void executeEqualizerMode(void) {
    // TODO: Equalizer mode implementation.
}

//=================================================================
// Getters y Setters
//=================================================================

void setMode(Mode const mode) {
    SYSTEM.mode = mode;
    SYSTEM.flag_ModeConfigured = RESET;
}

void setFilter(Filter const filter) {
    SYSTEM.filter = filter;
}

void clearFilter(void) {
    setFilter(passthrough);
}
