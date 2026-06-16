#pragma once

#include <stdint.h>

#define EQ_BANDS_N 8 /**< Number of frequency bands for EQ */

/**< User-configured values of EQ bands in a 0-255 scale **/
extern uint8_t eq_bands[EQ_BANDS_N];

void eqMode_registerHooks(void);
