#include "gpdma/gpdma.h"

#include "lpc17xx_gpdma.h"
#include "lpc_types.h"

/* ---------------------------------------------------------------------------
 * DMA ping-pong index
 *
 * fft_half_ready is toggled by the CH7 terminal-count interrupt.
 * It tells dsp_FFT() which half of FFT_SOURCE_BUFFER_TIME to consume.
 * Initialised to 1 so that the first toggle (1 -> 0) correctly selects
 * the first half, which is always the one filled by the initial CFG
 * transfer.
 * ---------------------------------------------------------------------------
 */
volatile int fft_half_ready = 1;

/* ---------------------------------------------------------------------------
 * Buffer for ADC samples (double-buffered)
 *
 * The GPDMA writes directly into this array using a two-entry linked
 * list that alternates between [0..PERIOD) and [PERIOD..2*PERIOD).
 * ---------------------------------------------------------------------------
 */
volatile uint16_t FFT_SOURCE_BUFFER_TIME[2 * PERIOD];

/* ---------------------------------------------------------------------------
 * Synchronisation flag – set by the CH7 TC interrupt, consumed by
 * system_executeRealTimeMode().
 * ---------------------------------------------------------------------------
 */
FlagStatus flag_bufferReadyforFFT = RESET;

/* ---------------------------------------------------------------------------
 * GPDMA channel-7 configuration
 *
 * Channel 7 transfers PERIOD halfwords from the ADC data register into
 * FFT_SOURCE_BUFFER_TIME.  A two-entry linked list alternates between
 * the first and second half of the buffer so that the CPU can process
 * one half while the DMA fills the other.
 * ---------------------------------------------------------------------------
 */

/* Forward declarations for the circular LLI chain. */
extern GPDMA_LLI_T adc_secondHalf_LLI;

/** LLI that writes the first half of the double buffer. */
GPDMA_LLI_T adc_firstHalf_LLI = {
    .srcAddr = (uint32_t)&LPC_ADC->ADGDR,
    .dstAddr = (uint32_t)&FFT_SOURCE_BUFFER_TIME[0],
    .nextLLI = (uint32_t)&adc_secondHalf_LLI,
    .control = (PERIOD | 7 << 12 | 7 << 15 | 1 << 18 | 1 << 21 | 1 << 27 | 1 << 31),
};

/** LLI that writes the second half of the double buffer. */
GPDMA_LLI_T adc_secondHalf_LLI = {
    .srcAddr = (uint32_t)&LPC_ADC->ADGDR,
    .dstAddr = (uint32_t)&FFT_SOURCE_BUFFER_TIME[PERIOD],
    .nextLLI = (uint32_t)&adc_firstHalf_LLI,
    .control = (PERIOD | 7 << 12 | 7 << 15 | 1 << 18 | 1 << 21 | 1 << 27 | 1 << 31),
};

/** Source endpoint: ADC (no increment, halfword, burst = 1). */
const GPDMA_Endpoint_T adc_sourceCH7 = {
    .width = GPDMA_HALFWORD,
    .burst = GPDMA_BSIZE_1,
    .increment = DISABLE,
};

/** Destination endpoint: memory (increment, halfword, burst = 1). */
const GPDMA_Endpoint_T adc_destCH7 = {
    .width = GPDMA_HALFWORD,
    .burst = GPDMA_BSIZE_1,
    .increment = ENABLE,
};

/**
 * Channel 7 configuration.
 *
 * The initial transfer writes PERIOD samples to the first half of
 * FFT_SOURCE_BUFFER_TIME; subsequent transfers alternate via the
 * linked-list chain above.
 */
GPDMA_Channel_CFG_T adc_buffer_CH_CFG = {
    .channelNum = GPDMA_CH_7,
    .transferSize = PERIOD,
    .type = GPDMA_P2M,
    .srcMemAddr = (uint32_t)&LPC_ADC->ADGDR,
    .dstMemAddr = (uint32_t)&FFT_SOURCE_BUFFER_TIME[0],
    .srcConn = GPDMA_ADC,
    .src = adc_sourceCH7,
    .dst = adc_destCH7,
    .intTC = ENABLE,
    .linkedList = (uint32_t)&adc_secondHalf_LLI,
};

/* ---------------------------------------------------------------------------
 * GPDMA initialisation
 *
 * Only channel 7 is needed: ADC -> FFT_SOURCE_BUFFER_TIME.
 * Channel 6 (previously used for an intermediate copy) is eliminated.
 * ---------------------------------------------------------------------------
 */
void gpdma_init(void) {
    GPDMA_Init();
    gpdma_setupChannelForADC(adc_buffer_CH_CFG);
    NVIC_EnableIRQ(DMA_IRQn);
}

void gpdma_setupChannelForADC(GPDMA_Channel_CFG_T cfg) {
    GPDMA_SetupChannel(&cfg);
    GPDMA_ChannelStart(GPDMA_CH_7);
}

/* ---------------------------------------------------------------------------
 * GPDMA interrupt handler
 *
 * Only CH7 is active.  On each terminal-count interrupt we toggle the
 * half-ready indicator and signal the main loop.
 * ---------------------------------------------------------------------------
 */
void gpdma_irq_handler(void) {
    if (GPDMA_IntGetStatus(GPDMA_INTTC, GPDMA_CH_7)) {
        fft_half_ready = !fft_half_ready;
        flag_bufferReadyforFFT = SET;
    }
}
