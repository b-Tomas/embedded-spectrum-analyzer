#pragma once

#include "lpc17xx_gpdma.h"
#include "lpc_types.h"

/**
 * @brief buffer size.
 */
#define BUFFER_SIZE 1024

/**
 * @brief FFT buffer fullied flag.
 */
extern FlagStatus flag_bufferReadyforFFT;

/**
 * @brief  Memory address for the double-buffer. Used for real tieme configuration.
 * switching btween theirselft, the free buffer is used as source for FFT(...), will be transferred
 * by DMA
 * @warning. DISCUSS: El valor del ADC se va cargando en un primer buffer, cuando se llena, mediante
 * una interrupción, inicia a transferir al buffer que usa FFT(...). Mientras tanto se carga el
 * segundo buffer, no estoy seguro si realmente es suficiente para que termine FFT(...) sin
 * que ingresen nuevos datos del ADC. 1024 datos / ADC_RATE => 1204/32Khz = 0.03125 s. Esto es lo
 * que tarda en llenase el buffer conectado al ADC
 */
extern volatile uint32_t FIRST_BUFFER_ADDRESS;
extern volatile uint32_t SECOND_BUFFER_ADDRESS;

/**
 * @brief  Memory address for the result of FFT(...)
 */
extern volatile uint32_t DSP_FFT_RESULT;

/**
 * @brief Real time channel configuration structs.
 */
extern GPDMA_Channel_CFG_T ChannelConfig_adc_buffer;
extern GPDMA_Channel_CFG_T ChannelConfig_buffer_FFT;
