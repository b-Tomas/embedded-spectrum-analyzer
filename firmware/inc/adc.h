#if !defined(ADC_H)
#define ADC_H

#include "lpc17xx_adc.h"

/* @brief ADC config when mode is in real time.
 */
void ADC_Mode1Configuration(void);

/* @brief ADC config when mode is in noise sampling.
 */
void ADC_Mode2Configuration(void);

/* @brief for ensure that config for real time has been run only once
 */
Bool ADC_Mode1Configurated(void);

/* @brief for ensure that config for noise sampling has been run only once
 */
Bool ADC_Mode1Configurated(void);

#endif // ADC_H