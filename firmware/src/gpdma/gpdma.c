#include "gpdma/gpdma.h"

#include "lpc17xx_gpdma.h"
#include "lpc_types.h"

//**< Arrayfor the filterH used in FFT */
volatile int16_t FILTER_H[PERIOD];

int16_t MAGNITUDE;

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
volatile uint32_t FIRST_BUFFER_ADDRESS;
volatile uint32_t SECOND_BUFFER_ADDRESS;

FlagStatus flag_bufferReadyforFFT = RESET;

/** Addresses for the input and output FFT buffers  */
volatile uint16_t FFT_SOURCE_BUFFER_TIME[PERIOD];
volatile int32_t DSP_FFT_RESULT_RE[PERIOD];
volatile int32_t DSP_FFT_RESULT_IM[PERIOD];

/** Address for the output IFFT buffers  */
volatile int32_t DSP_IFFT_RESULT[PERIOD];

/** Forward declarations for circular LLI linked lists */
extern GPDMA_LLI_T adc_firstBuffer_LLI;
extern GPDMA_LLI_T secondBuffer_FFT_LLI;

/**
 * Channel 7 configuration.
 * This channel transfers the ADC output to a memory buffer. It makes PERIOD trnansfers
 * until reconfigure, Once completed, it generates an interrupt [DISCUSS:] At that point,
 * we transfer the buffer (via DMA) to the one used by the FFT function.
 */

GPDMA_LLI_T adc_secondBuffer_LLI = {
    .srcAddr = (int32_t)&LPC_ADC->ADGDR,
    .dstAddr = (int32_t)&SECOND_BUFFER_ADDRESS,
    .nextLLI = (uint32_t)&adc_firstBuffer_LLI,
    .control = (PERIOD | 7 << 12 | 7 << 15 | 1 << 18 | 1 << 21 | 1 << 27 | 1 << 31),
};

GPDMA_LLI_T adc_firstBuffer_LLI = {
    .srcAddr = (uint32_t)&LPC_ADC->ADGDR,
    .dstAddr = (uint32_t)&FIRST_BUFFER_ADDRESS,
    .nextLLI = (uint32_t)&adc_secondBuffer_LLI,
    .control = (PERIOD | 7 << 12 | 7 << 15 | 1 << 18 | 1 << 21 | 1 << 27 | 1 << 31),
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

GPDMA_Channel_CFG_T adc_buffer_channelCfg = {
    .channelNum = GPDMA_CH_7,
    .transferSize = PERIOD,
    .type = GPDMA_P2M,
    .srcMemAddr = (uint32_t)&LPC_ADC->ADGDR,
    .dstMemAddr = (uint32_t)&FIRST_BUFFER_ADDRESS,
    .srcConn = GPDMA_ADC,
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
    .dstAddr = (uint32_t)&FFT_SOURCE_BUFFER_TIME,
    .nextLLI = (uint32_t)&secondBuffer_FFT_LLI,
    .control = (PERIOD | 1 << 18 | 1 << 21 | 1 << 26 | 1 << 27 | 1 << 31),
};

GPDMA_LLI_T secondBuffer_FFT_LLI = {
    .srcAddr = (uint32_t)&SECOND_BUFFER_ADDRESS,
    .dstAddr = (uint32_t)&FFT_SOURCE_BUFFER_TIME,
    .nextLLI = (uint32_t)&firstBuffer_FFT_LLI,
    .control = (PERIOD | 1 << 18 | 1 << 21 | 1 << 26 | 1 << 27 | 1 << 31),
    /** Once completed raise the flag that triggers the FFT() */
};

GPDMA_Channel_CFG_T ChannelConfig_buffer_FFT = {
    .channelNum = GPDMA_CH_6,
    .transferSize = PERIOD,
    .type = GPDMA_M2M,
    .srcMemAddr = (uint32_t)&FIRST_BUFFER_ADDRESS,
    .dstMemAddr = (uint32_t)&FFT_SOURCE_BUFFER_TIME,
    .src = endpointCH6,
    .dst = endpointCH6,
    .intTC = ENABLE,
    // TODO: the mode 1 handler should be switching the buffers
    .linkedList = 0 //(uint32_t)&secondBuffer_FFT_LLI,
};

void DMA_IRQHandler(void) {
    if (GPDMA_IntGetStatus(GPDMA_INTTC, GPDMA_CH_7)) {
        /** TODO: Sequence upon completion adc-buffer transfer */
    }
    if (GPDMA_IntGetStatus(GPDMA_INTTC, GPDMA_CH_6)) {
        // stop transfer
        /** TODO: Sequence upon completioon buffer to FFT's buffer
         */
        flag_bufferReadyforFFT = SET;
    }
    /* */
}
