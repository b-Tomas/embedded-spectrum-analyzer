#include "system/system.h"

#include "LPC17xx.h"
#include "adc/adc.h"
#include "dsp/dsp.h"
#include "gpdma/gpdma.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"
#include "lpc_types.h"
#include "system/nonCFG.h"

#include <stddef.h>
#include <stdint.h>

/** @brief Global orchestrator instance. */
System SYSTEM;

FlagStatus flag_readyToDisplay = RESET;

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

    /**
     * ------------------------------------------------------------------------------------
     *                     GPDMA non-configurable setting
     * ------------------------------------------------------------------------------------
     */

    /**< This channel transfers the ADC output to a memory buffer */
    GPDMA_SetupChannel(&adc_buffer_channelCfg);
    GPDMA_ChannelStart(GPDMA_CH_7);

    /**< This channel transfers the previous buffer to the one used by the FFT implementation  */
    GPDMA_SetupChannel(&ChannelConfig_buffer_FFT);

    /**< This channel transfers the FFT result to ****  */
    /** TODO: set up channel, dmaCfg struct, etc */

    /**
     * ------------------------------------------------------------------------------------
     *                     Display non-configurable settings
     * ------------------------------------------------------------------------------------
     */

    /**< TIMER1 configuration */
    TIM_InitTimer(LPC_TIM1, &tim1_dspl_cfg);
    TIM_ConfigMatch(LPC_TIM1, &tim1_dspl_matchcfg);
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

        dsp_FFT((uint16_t*)FFT_SOURCE_BUFFER_TIME, (int32_t*)DSP_FFT_RESULT_RE,
                (int32_t*)DSP_FFT_RESULT_IM);

        /** TODO: Make a new version of applyFilter() */
        dsp_IFFT((int32_t*)DSP_FFT_RESULT_RE, (int32_t*)DSP_FFT_RESULT_IM,
                 (int32_t*)DSP_IFFT_RESULT);

        if (flag_readyToDisplay) {
            flag_readyToDisplay = RESET;

            uint8_t bars[N_BARS];
            dsp_compressSignal((const int32_t*)DSP_IFFT_RESULT, bars);
            update_bars(bars);
        }

        flag_bufferReadyforFFT = RESET;

    } else if (flag_bufferReadyforFFT) {
        uint32_t adcSample = ADC_ChannelGetData(ADC_CHANNEL_0);
        DAC_UpdateValue(adcSample);

        if (flag_readyToDisplay) {
            flag_readyToDisplay = RESET;

            // DSP_IFFT_RESULT is unused in passthrough — repurpose as scratch.
            volatile int32_t* signal = DSP_IFFT_RESULT;
            for (int i = 0; i < PERIOD; i++)
                signal[i] = (int32_t)FFT_SOURCE_BUFFER_TIME[i] - ADC_CENTER;

            uint8_t bars[N_BARS];
            dsp_compressSignal((const int32_t*)DSP_IFFT_RESULT, bars);
            update_bars(bars);
        }

        flag_bufferReadyforFFT = RESET;
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
