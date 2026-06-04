#ifndef FFT_H
#define FFT_H
//version optimizada como sobra ram precomputo todo el bit reverse consumia CPU
#include <stdint.h>

#define LOG2_PERIOD  10
#define PERIOD       (1 << LOG2_PERIOD)
#define ADC_MAX      4095
#define ADC_CENTER   2048
#define Q15_ONE      32767

void FFT        (uint16_t* sourceT, int32_t* sourceR, int32_t* sourceI);
void IFFT       (int32_t*  sourceR, int32_t* sourceI, int32_t* resultT);
void buildFilter(int16_t*  filterH, int binLow, int binHigh, int16_t magnitude);
void applyFilter(int32_t*  OmR,     int32_t*  OmI, const int16_t* filterH);

#endif // FFT_H