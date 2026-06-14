#include "system/system.h"

#include "LPC17xx.h"
#include "adc/adc.h"
#include "fft/fft.h"
#include "gpdma/gpdma.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#include "lpc_types.h"

#include <stddef.h>
#include <stdint.h>

/** @brief Global orchestrator instance. */
System SYSTEM;

FlagStatus flag_buildPassThroughFilter = RESET;
FlagStatus flag_buildNoiseSuppressionFilter = RESET;
FlagStatus flag_buildPassLowFilter = RESET;
FlagStatus flag_buildPassHighFilter = RESET;
FlagStatus flag_buildPassBandFilter = RESET;
FlagStatus flag_buildRejectBandFilter = RESET;

void system_Init(Mode mode) {
    system_setMode(mode);
    GPDMA_Init();
    ADC_Init(ADC_RATE);
    DAC_Init();
    // TODO: Call all the default peripheral config.
}

void system_nonConfigurableSetting(void) {
    /** ADC is always on, should not be modified on the changes mode*/
    ADC_StartCmd(ADC_START_CONTINUOUS);
    ADC_PinConfig(ADC_CHANNEL_0);
    ADC_ChannelEnable(ADC_CHANNEL_0);
    ADC_BurstEnable();
    ADC_PowerUp();

    /** GPDMA non-configurable setting */

    /**< This channel transfers the ADC output to a memory buffer */
    GPDMA_SetupChannel(&ChannelConfig_adc_buffer);
    GPDMA_ChannelStart(GPDMA_CH_7);

    /**< This channel transfers the previous buffer to the one used by the FFT implementation  */
    GPDMA_SetupChannel(&ChannelConfig_buffer_FFT);

    /**< This channel transfers the FFT result to ****  */
    /** TODO: set up channel, dmaCfg struct, etc */
}

void system_ConfigureSetting_RealTimeMode(void) {}

void system_ConfigureSetting_NoiseSamplingMode() {
    // TODO: ADC reconfig, GPDMA re-config, DISPLAY re-config?
}

void system_ConfigureSetting_EqualizerMode() {
    // TODO: KEYBOARD re-config, ADC-stop? GPDMA-reconfig (select filer), DISPLAY re-config
}

void system_StartRealTimeMode(void) {

    if (SYSTEM.filter != passthrough && flag_bufferReadyforFFT) {

        FFT((uint16_t*)FFT_SOURCE_BUFFER_TIME, (int32_t*)DSP_FFT_RESULT_RE,
            (int32_t*)DSP_FFT_RESULT_IM);

        /**< Supongo que la flag se levanta en el handler del teclado al estar en modo 1 al
         * menos que sea el de noiseSuppression que se tendria que levantar en un handler
         * diferente */
        if (flag_buildPassThroughFilter || flag_buildNoiseSuppressionFilter ||
            flag_buildPassLowFilter || flag_buildPassHighFilter || flag_buildPassBandFilter ||
            flag_buildRejectBandFilter) {

            switch (SYSTEM.filter) {
            case passthrough:
                /**< Pass all frequencies: bins 0–511, full gain */
                buildFilter(FILTER_H, 0, (PERIOD / 2) - 1, Q15_ONE);
                flag_buildPassThroughFilter = RESET;
                break;
            case noiseSuppression:

                /**< TODO: No se como aplicar buildFilter() para una muestra de ruido*/
                flag_buildNoiseSuppressionFilter = RESET;
                break;
            case lowPass:

                /**< Pass 0–500 Hz → bins 0–15  (500/32 = 15.6 ≈ 15) */
                buildFilter(FILTER_H, 0, 15, MAGNITUDE);
                flag_buildPassLowFilter = RESET;
                break;
            case highPass:

                /**< Pass 4 kHz–16 kHz → bins 125–511  (4000/32 = 125) */
                buildFilter(FILTER_H, 125, (PERIOD / 2) - 1, MAGNITUDE);
                flag_buildPassHighFilter = RESET;
                break;
            case bandPass:

                /**< Pass 500 Hz–4 kHz → bins 16–124 */
                buildFilter(FILTER_H, 16, 124, MAGNITUDE);
                flag_buildPassBandFilter = RESET;
                break;
            case bandReject:

                /**< Reject 500 Hz–4 kHz → bins 16–124, magnitude = 0
                 *  DISCUSS: si hacemos que el usuario puedea seleccionar las bandas
                 * Inside band = 0, outside band = Q15_ONE */
                buildFilter(FILTER_H, 16, 124, 0);
                flag_buildRejectBandFilter = RESET;
                break;
            }
        }

        applyFilter((int32_t*)DSP_FFT_RESULT_RE, (int32_t*)DSP_FFT_RESULT_IM, (int16_t*)FILTER_H);
        IFFT((int32_t*)DSP_FFT_RESULT_RE, (int32_t*)DSP_FFT_RESULT_IM, (int32_t*)DSP_IFFT_RESULT);

        /**< Show in the display */
        /** TODO: Mostrar por los displays */

        flag_bufferReadyforFFT = RESET;

    } else {
        uint32_t adcSample = ADC_ChannelGetData(ADC_CHANNEL_0);
        DAC_UpdateValue(adcSample);
        /** display the signal
         * Tomo el ultimo valor del ADC y lo cargo como una barra, desplazo el valor anterior
         * hacia la derecha?
         */
    }
}

void system_StartNoiseSamplingMode(void) {
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

void system_StartEqualizerMode(void) {
    /**
     * TODO: Equalizer mode implementation.
     *
     * input the number -> display it -> confirm -> save it (in EQUALIZER)
     *
     */
}

//=================================================================
// Getters y Setters
//=================================================================

void system_setMode(Mode mode) {
    SYSTEM.mode = mode;
    SYSTEM.flag_ModeConfigured = RESET;
}

void setFilter(Filter filter) {
    SYSTEM.filter = filter;

    if (filter == passthrough) {
        /**< Stop DMA data handler. When passthrough is set, The system dont aplly FFT and Inverse
         * FFT, just update the DAC with the ADC new sample
         * Pause the channels related with ADC-Buffer trnasfers
         */
        GPDMA_ChannelGracefulStop(GPDMA_CH_7);
        GPDMA_ChannelGracefulStop(GPDMA_CH_6);
    } else {

        GPDMA_ChannelResume(GPDMA_CH_7);
        GPDMA_ChannelResume(GPDMA_CH_6);
    }
}

void clearFilter(void) {
    setFilter(passthrough);
}