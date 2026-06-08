#pragma once

#include "lpc17xx_gpdma.h"
#include "lpc_types.h"

/**
 * @brief buffer size.
 */
#define TRANSFER_SIZE_wBURST256 4

/**
 * @brief flag that indicates that the FFT input buffer is ready.
 */
extern FlagStatus flag_bufferReadyforFFT;

/**
 * @brief Memory address for the FFT input buffer
 */
extern volatile uint32_t FFT_SOURCE_BUFFER_TIME;

/**
 * @brief  Memory addresses for the results of the FFT (real and imaginary part)
 */
extern volatile uint32_t DSP_FFT_RESULT_RE;
extern volatile uint32_t DSP_FFT_RESULT_IM;

/**
 * @brief  Memory addresses for the source of the IFFT
 */
extern volatile uint32_t IFFT_SOURCE_BUFFER_RE;
extern volatile uint32_t IFFT_SOURCE_BUFFER_IM;

/**
 * @brief  Memory address for the result of IFFT
 */
extern volatile uint32_t DSP_IFFT_RESULT;

/**
 * @brief Real time channel configuration structs.
 */
extern GPDMA_Channel_CFG_T ChannelConfig_adc_buffer;
extern GPDMA_Channel_CFG_T ChannelConfig_buffer_FFT;
