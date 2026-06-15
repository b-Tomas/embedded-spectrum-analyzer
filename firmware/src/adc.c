#include "adc/adc.h"

#include "lpc17xx_adc.h"

void adc_init(void) {
    ADC_Init(ADC_RATE);
    ADC_StartCmd(ADC_START_CONTINUOUS); /** DISCUSS: Podriamos hacer que inicie al termiar la FFT*/
    ADC_PinConfig(ADC_CHANNEL_0);
    ADC_ChannelEnable(ADC_CHANNEL_0);
    ADC_BurstEnable();
    ADC_PowerUp();
}