/**
 * Graphics engine. Requires display_init(); to be called beforehand
 */

#pragma once
#include <stdint.h>

// TODO(b-Tomas): wire with some global value for the amount of bands to show
#define N_BARS 10

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
