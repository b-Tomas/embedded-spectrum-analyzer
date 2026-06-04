#pragma once

#include "system.h"

#include "lpc_types.h"

void systemInit(Mode mode) {
    setMode(mode);
    // TODO: Call all the default peripheral config.
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

void changeEqualizer(const uint32_t* newEQBands) {
    // TODO: rewrite the EQUALIZER with the array that cointains the new EQ values.
}

//=================================================================
// Getters y Setters
//=================================================================

void setMode(Mode mode) {
    SYSTEM.mode = mode;
    SYSTEM.flag_ModeConfigured = RESET;
}

void setFilter(Filter filter) {
    SYSTEM.filter = filter;
}

void clearFilter(void) {
    setFilter(passthrough);
}