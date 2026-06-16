#pragma once

#include <stdint.h>

#define EQ_BANDS_N 10 /**< Number of frequency bands for EQ */

/**< User-configured values of EQ bands in a 0-255 scale **/
extern uint8_t eq_bands[EQ_BANDS_N];

/**
 * @brief The implementation of the equalizer mode.
 * @details UI experience to configure the gains bands
 * @note Must update EQUALIZER.
 */
void system_tickEqualizerMode(void);

/**
 * @brief Process a key press in the context of the equalizer mode
 */
void system_processKeyEqualizerMode(char key);

/**
 * @brief System config for equalizer mode.
 */
void system_ConfigureSetting_EqualizerMode(void);
