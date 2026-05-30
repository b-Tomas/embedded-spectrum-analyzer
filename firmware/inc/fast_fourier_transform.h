
#if !defined(FAST_FOURIER_TRASNSFORM_H)
#define FAST_FOURIER_TRASNSFORM_H

#include "lpc17xx.h"

#include <math.h>
#include <stdint.h>

#define LOG2_PERIOD

/*  @brief Algorithm for calculating the fast Fourier transform
 *   @param adcSamples Array input with the ADC's samples [0-2047]
 *   @param reDestination Memory address to save the real part result
 *   @param imDestination Memory address to save the imaginary part result
 */
void fastFourierTransform(uint16_t adcSamples, uint32_t* reDestination, uint32_t imDestination);

#endif // FAST_FOURIER_TRASNSFORM_H
