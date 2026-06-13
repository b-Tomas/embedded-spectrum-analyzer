#pragma once

#include "fft.h"

#include <stdint.h>

/** Twiddle factors: tw_cos[k] = round(cos(2π·k/N) * 32767), N = PERIOD. */
extern const int16_t tw_cos[1024];

/** Precomputed bit-reversal permutation for N = PERIOD. */
extern const uint16_t bit_rev_table[1024];
