
#if !defined(FAST_FOURIER_TRASNSFORM_H)
#define FAST_FOURIER_TRASNSFORM_H

#include <stdint.h>

// defines uses in Fast Fourier Transform
#define LOG2_PERIOD
#define inputBuffer
#define outputBuffer

/*  @brief Algorithm for calculating the fast Fourier transform
 *   @param adcSamples Array input with the ADC's samples [0-2047]
 *   @param reDestination Memory address to save the real part result
 *   @param imDestination Memory address to save the imaginary part result
 */
void fastFourierTransform(uint16_t adcSamples, uint32_t* reDestination, uint32_t imDestination);

#endif // FAST_FOURIER_TRASNSFORM_H
