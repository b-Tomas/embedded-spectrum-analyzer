#ifndef FFT_H
#define FFT_H

#include <stdint.h>

#define LOG2_PERIOD  10
#define PERIOD       (1 << LOG2_PERIOD)
#define ADC_MAX      4095
#define ADC_CENTER   2048

// Q15: 1.0 = 32767, 0.0 = 0
// Usar Q15_ONE como magnitude = ganancia unitaria (sin atenuar)
#define Q15_ONE  32767

// ─── Prototipos ───────────────────────────────────────────────────────────────
/**
 * FFT
 *
 *   Calcula la Transformada Rápida de Fourier (FFT) de un conjunto de muestras de tiempo.
 *
 *   @param sourceT  Array de entrada con muestras de tiempo (uint16_t)
 *   @param sourceR  Array de entrada con parte real del espectro (int64_t)
 *   @param sourceI  Array de entrada con parte imaginaria del espectro (int64_t)
 */
void FFT (uint16_t* sourceT, int64_t* sourceR, int64_t* sourceI);

/**
 * IFFT
 *
 *   Calcula la Transformada Rápida de Fourier Inversa (IFFT) de un conjunto de muestras del espectro.
 *
 *   @param sourceR  Array de entrada con parte real del espectro (int64_t)
 *   @param sourceI  Array de entrada con parte imaginaria del espectro (int64_t)
 *   @param resultT  Array de salida con muestras de tiempo (int32_t)
 */
void IFFT(int64_t*  sourceR, int64_t* sourceI, int32_t* resultT);

/**
 * buildFilter
 *
 *   Construye la respuesta en frecuencia H[k] sobre filterH[PERIOD].
 *   El filtro es siempre simétrico (espejo en PERIOD-k) para que la
 *   IFFT produzca una señal real.
 *
 *   Funciona como pasa-banda con magnitud parametrizable:
 *     - Los bins dentro de [binLow .. binHigh] reciben 'magnitude'
 *     - Los bins fuera de ese rango reciben 'Q15_ONE - magnitude'
 *
 *   Casos particulares:
 *     Pasa bajo  → binLow=0,         binHigh=corte,  magnitude=Q15_ONE
 *     Pasa alto  → binLow=corte,     binHigh=N/2,    magnitude=Q15_ONE
 *     Pasa banda → binLow=f1,        binHigh=f2,     magnitude=Q15_ONE
 *     Rechaza banda → binLow=f1,     binHigh=f2,     magnitude=0
 *     Atenuación parcial → magnitude entre 0 y Q15_ONE
 *
 *   @param filterH    Array destino [PERIOD]
 *   @param binLow     Bin inferior de la banda (0 .. PERIOD/2)
 *   @param binHigh    Bin superior de la banda (0 .. PERIOD/2)
 *   @param magnitude  Ganancia dentro de la banda en Q15 (0=bloquea, Q15_ONE=pasa)
 */
void buildFilter(int64_t* filterH, int binLow, int binHigh, int16_t magnitude);

/**
 * applyFilter
 *
 *   Multiplica el espectro complejo (OmR + j*OmI) por el filtro real H[k].
 *   Modifica OmR y OmI in-place.
 *
 *   @param OmR     Parte real del espectro
 *   @param OmI     Parte imaginaria del espectro
 *   @param filterH Respuesta en frecuencia (construida con buildFilter)
 */
void applyFilter(int64_t* OmR, int64_t* OmI, const int64_t* filterH);

// Función legacy
void substractArraysFrec(int64_t* array1, int64_t* array2);

#endif // FFT_H