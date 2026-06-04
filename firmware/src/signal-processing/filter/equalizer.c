#include "signal-processing/filter/equalizer.h"

#include <stdint.h>

uint32_t EQUALIZER[N_BANDS];

void equalizer_saveEqualization(const uint32_t* newEqualizer) {
    if (!newEqualizer) {
        return;
    }

    for (uint32_t band = 0; band < N_BANDS; ++band) {
        EQUALIZER[band] = newEqualizer[band];
    }
}

void equalizer_saveOneBand(int index, uint32_t indexBandValue) {
    if (index > N_BANDS || index < 0) {
        return;
    }

    EQUALIZER[index] = indexBandValue;
    return;
}