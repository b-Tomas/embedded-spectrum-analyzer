#if !defined(ADC_H)
#define ADC_H

/* @brief ADC config when mode is in real time.
 */
void ADC_setRealTimeMode(void);

/* @brief ADC config when mode is in noise sampling.
 */
void ADC_setNoiseSamplingMode(void);

#endif // ADC_H