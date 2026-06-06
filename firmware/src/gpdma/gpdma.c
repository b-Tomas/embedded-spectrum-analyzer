#include "gpdma/gpdma.h"

#include "lpc17xx_gpdma.h"
#include "lpc_types.h"

uint32_t FIRST_BUFFER_ADDRESS;
uint32_t SECOND_BUFFER_ADDRESS;
uint32_t FFT_BUFFER_ADDRESS; /** simbolic */

/**
 * @brief LLI structs for ADC - Buffer.
 */
GPDMA_LLI_T adc_firstBuffer_LLI;
GPDMA_LLI_T adc_secondBuffer_LLI;

/**
 * @brief LLI structs for buffer - FFT.
 */
GPDMA_LLI_T firstBuffer_FFT_LLI;
GPDMA_LLI_T secondBuffer_FFT_LLI;

/**
 * Channel 7 configuration.
 * This channel transfers the ADC output to a memory buffer. It makes PERIOD trnansfers
 * until reconfigure, Once completed, it generates an interrupt [DISCUSS:] At that point,
 * we transfer the buffer (via DMA) to the one used by the FFT function.
 */

GPDMA_LLI_T adc_secondBuffer_LLI = {
    .srcAddr = (uint32_t)&LPC_ADC->ADGDR,
    .dstAddr = (uint32_t)&SECOND_BUFFER_ADDRESS,
    .nextLLI = (uint32_t)&adc_firstBuffer_LLI,
    .control = (BUFFER_SIZE | 1 << 18 | 1 << 21 | 1 << 27 | 1 << 31),
};

GPDMA_LLI_T adc_firstBuffer_LLI = {
    .srcAddr = (uint32_t)&LPC_ADC->ADGDR,
    .dstAddr = (uint32_t)&FIRST_BUFFER_ADDRESS,
    .nextLLI = (uint32_t)&adc_secondBuffer_LLI,
    .control = (BUFFER_SIZE | 1 << 18 | 1 << 21 | 1 << 27 | 1 << 31),
};

const GPDMA_Endpoint_T sourceCH7 = {
    .width = GPDMA_HALFWORD,
    .burst = GPDMA_BSIZE_1,
    .increment = DISABLE,
};

const GPDMA_Endpoint_T destinationCH7 = {
    .width = GPDMA_HALFWORD,
    .burst = GPDMA_BSIZE_1,
    .increment = ENABLE,
};

GPDMA_Channel_CFG_T ChannelConfig_adc_buffer = {
    .channelNum = GPDMA_CH_7,
    .transferSize = BUFFER_SIZE,
    .type = GPDMA_P2M,
    .srcMemAddr = (uint32_t)&LPC_ADC->ADGDR,
    .dstMemAddr = (uint32_t)&FIRST_BUFFER_ADDRESS,
    .dstConn = GPDMA_ADC,
    .src = sourceCH7,
    .dst = destinationCH7,
    .intTC = ENABLE,
    .linkedList = (uint32_t)&adc_secondBuffer_LLI,
};

/** Channel 6 configuration
TODO(samuel): connection between the free buffer to the one used by FFT(...)
*/

const GPDMA_Endpoint_T endpointCH6 = {
    .width = GPDMA_HALFWORD,
    .burst = GPDMA_BSIZE_1,
    .increment = ENABLE,
};

GPDMA_LLI_T firstBuffer_FFT_LLI = {
    .srcAddr = (uint32_t)&FIRST_BUFFER_ADDRESS,
    .dstAddr = (uint32_t)&FFT_BUFFER_ADDRESS, // temp
    .nextLLI = (uint32_t)&secondBuffer_FFT_LLI,
    .control = (BUFFER_SIZE | 1 << 18 | 1 << 21 | 1 << 26 | 1 << 27 | 1 << 31),
};

GPDMA_LLI_T secondBuffer_FFT_LLI = {
    .srcAddr = (uint32_t)&SECOND_BUFFER_ADDRESS,
    .dstAddr = (uint32_t)&FFT_BUFFER_ADDRESS, // temp
    .nextLLI = (uint32_t)&firstBuffer_FFT_LLI,
    .control = (BUFFER_SIZE | 1 << 18 | 1 << 21 | 1 << 26 | 1 << 27 | 1 << 31),
    /** Once completed raise the flag that triggers the FFT() */
};

GPDMA_Channel_CFG_T ChannelConfig_buffer_FFT = {
    .channelNum = GPDMA_CH_6,
    .transferSize = BUFFER_SIZE,
    .type = GPDMA_M2M,
    .srcMemAddr = (uint32_t)&FIRST_BUFFER_ADDRESS,
    .dstMemAddr = (uint32_t)&FFT_BUFFER_ADDRESS, // temp
    .src = endpointCH6,
    .dst = endpointCH6,
    .intTC = ENABLE,
    .linkedList = (uint32_t)&secondBuffer_FFT_LLI,
};

void GPDMA_IRQHandler(void) {
    if (GPDMA_IntGetStatus(GPDMA_INTTC, GPDMA_CH_7)) {
        /** TODO: Sequence upon completion adc-buffer transfer */
    }
    if (GPDMA_IntGetStatus(GPDMA_INTTC, GPDMA_CH_6)) {
        /** TODO: Sequence upon completioon buffer to FFT's buffer
         * Should rise a flag to trigger FFT.
         */
    }
    /* */
}