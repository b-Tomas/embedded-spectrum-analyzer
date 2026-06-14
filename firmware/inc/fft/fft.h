#pragma once

#include <stdint.h>

#define LOG2_PERIOD 10
#define PERIOD      (1 << LOG2_PERIOD)
#define ADC_MAX     4095
#define ADC_CENTER  2048
#define Q15_ONE     32767

/**
 * @brief Computes the in-place FFT of a real ADC input signal.
 *
 * @param sourceT  Input: time-domain ADC samples (uint16_t, length PERIOD).
 * @param resultR  Output: real part of the frequency-domain result (Q15 scaled).
 * @param resultI  Output: imaginary part of the frequency-domain result (Q15 scaled).
 */
void FFT(uint16_t* sourceT, int32_t* resultR, int32_t* resultI);

/**
 * @brief Computes the inverse FFT, recovering the time-domain signal.
 *
 * @param sourceR  Input: real part of the spectrum (output of FFT or filtered).
 * @param sourceI  Input: imaginary part of the spectrum.
 * @param resultT  Output: reconstructed time-domain signal, divided by PERIOD.
 */
void IFFT(int32_t* sourceR, int32_t* sourceI, int32_t* resultT);

/**
 * @brief Builds a rectangular frequency-domain filter with variable gain.
 *
 * Bins inside [binLow..binHigh] are set to @param magnitude; bins outside are set
 * to (Q15_ONE - magnitude). The filter is filled symmetrically so that IFFT
 * returns a real signal.
 *
 * @param filterH    Output buffer of length PERIOD (int16_t).
 * @param binLow     First bin of the passband (inclusive).
 * @param binHigh    Last bin of the passband (inclusive).
 * @param magnitude  Gain inside the band, in Q15 format (0..Q15_ONE).
 */
void buildFilter(int16_t* filterH, int binLow, int binHigh, int16_t magnitude);

/**
 * @brief Multiplies the complex spectrum by a real filter H[k], in-place.
 *
 * @param resultR     Real part of the spectrum (modified in-place).
 * @param resultI     Imaginary part of the spectrum (modified in-place).
 * @param filterH Filter coefficients in Q15 format, length PERIOD.
 */
void applyFilter(int32_t* resultR, int32_t* resultI, int16_t* filterH);
