#pragma once

#include "system.h"

#include "lpc_types.h"


void setMode(Mode mode) {
    SYSTEM.mode = mode;
    SYSTEM.flag_ModeRunned = RESET; 
}

Mode getPreviousMode() {
    return SYSTEM.previousMode;
}

void systemInit(Mode mode){
    SYSTEM.mode = mode;  
    //TODO: Call all the default peripheral config. 
}

void configRealTimeMode(void){
    //TODO: ADC re-config, GPDMA re-config, DISPLAY re-config 
} 

void configNoiseSamplingMode(void){
    //TODO: ADC reconfig, GPDMA re-config, DISPLAY re-config? 
}

void configEqualizerMode(void){
    //TODO: KEYBOARD re-config, ADC-stop? GPDMA-reconfig (select filer), DISPLAY re-config
}

void setPreviousMode(void){
    SYSTEM.mode = SYSTEM.previousMode; 
    SYSTEM.flag_ModeRunned = RESET; 
    return;
}

void changeEqualizer(const uint32_t* newEQBands){
    //TODO: rewrite the EQUALIZER with the array that cointains the new EQ values. 
}