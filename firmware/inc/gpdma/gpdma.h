#pragma once

#include "dsp/dsp.h"
#include "lpc17xx_gpdma.h"
#include "lpc_types.h"

#include <stdint.h>

/**
 * @brief buffer size (FFT).
 */
#define TRANSFER_SIZE_wBURST256 4

/**
 * @brief Memory addres for the buildFiler
 */
extern volatile int16_t FILTER_H[PERIOD];

/**
 * @brief Magnitude used in FFT
 * @note update by keyboard
 */
extern int16_t MAGNITUDE;

/**
 * @brief flag that indicates that the FFT input buffer is ready.
 */
extern FlagStatus flag_bufferReadyforFFT;

/**
 * @brief Memory address for the FFT input buffer
 */
extern volatile uint16_t FFT_SOURCE_BUFFER_TIME[PERIOD];

/**
 * @brief  Memory addresses for the results of the FFT (real and imaginary part)
 */
extern volatile int32_t DSP_FFT_RESULT_RE[PERIOD];
extern volatile int32_t DSP_FFT_RESULT_IM[PERIOD];

/**
 * @brief  Memory address for the result of IFFT
 */
extern volatile int32_t DSP_IFFT_RESULT[PERIOD];

/**
 * @brief Real time channel configuration structs.
 */
extern GPDMA_Channel_CFG_T adc_buffer_channelCfg;
extern GPDMA_Channel_CFG_T ChannelConfig_buffer_FFT;
