/**
 * Graphics engine. Requires display_init(); to be called beforehand
 */

#pragma once
#include "system/eq_mode.h"

#include <stdbool.h>
#include <stdint.h>

#define N_BARS 64 /**< Number of bars to show in a bar chart */

/**
 * @brief Initialize a bars graphic with all bars at 0
 */
void init_bars();

/**
 * Updates a bars graphic. Initialize with @ref init_bars first.
 *
 * @param newBars new value for the bars
 */
void update_bars(uint8_t const newBars[N_BARS]);

/**
 * Configuration UI for EQ bands
 *
 * NOTE(b-Tomas): yes, update_bars takes a px scal (0-64) and this takes a different scale. it is
 * what it is. I'll refactor for consistency if we have the time.
 *
 * @param newBands new band values to show in the graph in the 0-255 range
 * @param selectedBand band to highlight in the graph
 * @param selectedBandBaseOn whether the base of the band is on or off (for a blinking effect)
 */
void gfx_update_eq_bands(uint8_t const newBands[EQ_BANDS_N], uint8_t selectedBand,
                         bool selectedBandBaseOn);
