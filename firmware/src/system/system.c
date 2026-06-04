#include "system/system.h"

#include "lpc17xx_adc.h"
#include "lpc_types.h"
#include "signal-processing/equalizer.h"

#include <stdbool.h>

/** @brief Global orchestrator instance. */
System SYSTEM;

void systemInit(Mode mode) {
    setMode(mode);
    // TODO: Call all the default peripheral config.
}

void configRealTimeMode(void) {
    // TODO: ADC re-config, GPDMA re-config, DISPLAY re-config
    /* ADC */
}

void configNoiseSamplingMode(void) {
    // TODO: ADC reconfig, GPDMA re-config, DISPLAY re-config?
    ADC_DeInit();
}

void configEqualizerMode(void) {
    // TODO: KEYBOARD re-config, ADC-stop? GPDMA-reconfig (select filer), DISPLAY re-config
}

void executeRealTimeMode(void) {

    /** ADC configured in burst mode, needs to indicate when input buffer is filled */

    /**    if (input buffer not filled) return;
     *    - Apply Fourier Transform.
     *    - Aplly filter. exectuteFilter();
     *    - Apply inverse Fourier Transform.
     *    DISCUSS:Indirect? transfers the fullied signal to DAC and displays.
     */
}

void executeNoiseSamplingMode(void) {
    /** TODO: Noise-Sampling mode implementation
     *
     *  DISCUSS: maybe we need a flag for this]
     *   if (input buffer not filled) return;
     *    - Apply Fourier Transform.
     *    - Aplly filter. exectuteFilter();
     *    - Apply inverse Fourier Transform.
     *    - DISCUSS: Indirect? transfers the fullied signal to DAC and displays.
     */
}

void executeEqualizerMode(void) {
    /**
     * TODO: Equalizer mode implementation.
     *
     * input the number -> display it -> confirm -> save it (in EQUALIZER)
     * DISCUSS: de Keyboard so
     */
}

//=================================================================
// Getters y Setters
//=================================================================

void setMode(Mode mode) {
    SYSTEM.mode = mode;
    SYSTEM.flag_ModeConfigured = RESET;
}
