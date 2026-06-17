#pragma once

#include "dsp/dsp.h"
#include "lpc17xx_gpdma.h"
#include "lpc_types.h"

#include <stdint.h>

/**
 * @brief Ping-pong flag indicating which half of FFT_SOURCE_BUFFER_TIME is ready.
 *
 * Toggled by the GPDMA CH7 terminal-count interrupt.
 * 0 = first half [0..PERIOD), 1 = second half [PERIOD..2*PERIOD).
 */
extern volatile int flag_halfReady;

/**
 * @brief Flag that indicates that the FFT input buffer is ready.
 */
extern volatile FlagStatus flag_bufferReadyforFFT;

/**
 * @brief Memory address for the FFT input buffer (double-buffered).
 *
 * The GPDMA CH7 writes ADC samples directly into this buffer using
 * a ping-pong scheme:
 *   - Even transfers go to [0 .. PERIOD-1]
 *   - Odd  transfers go to [PERIOD .. 2*PERIOD-1]
 */
extern volatile uint16_t FFT_SOURCE_BUFFER_TIME[2 * PERIOD];

/**
 * @brief Initialise the GPDMA interface.
 */
void gpdma_init(void);

/**
 * @brief Set up channel 7 for ADC-to-memory transfers.
 */
void gpdma_setupChannelForADC(GPDMA_Channel_CFG_T cfg);

/**
 * @brief GPDMA interrupt handler (called by DMA_IRQHandler).
 */
void gpdma_irq_handler(void);
