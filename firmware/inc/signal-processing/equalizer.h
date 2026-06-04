#pragma once

#include "lpc_types.h"

/**
 * @brief The numbers of frequency bands being modified.
 */
#define N_BANDS 10

/**
 * @brief Tha values of gains in decibels for each frequency.
 * possible values: -12 to 12
 */
extern uint32_t EQUALIZER[N_BANDS];

/**
 * @brief Replace a EQUALIZER with new values.
 *
 * @param newEqualizer New values for EQUALIZER.
 */
void equalizer_saveEqualization(const uint32_t* newEqualizer);

/**
 * @brief Replace a specific band from EQUALIZER
 *
 * @param indexBandValue New value for the band.
 */
void equalizer_saveOneBand(int index, uint32_t indexBandValue);