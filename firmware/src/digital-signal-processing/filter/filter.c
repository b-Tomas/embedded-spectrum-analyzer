#include "digital-signal-processing/filter/filter.h"

/**
 * @brief Instance of filter
 */
Filter filter;

void setFilter(Filter _filter) {
    filter = _filter;
}

void clearFilter(void) {
    setFilter(passthrough);
}

void executeFilter(void) {
    if (filter == passthrough) {
        return;
    } else if (filter == noiseSuppression) {
        /**  TODO: Apply the result of noise suppression mode.*/
    } else {
        /** TODO: Apply the equalization saved in EQUALIZER. i believe @emanuelpinque-cmd has
         * already implemented part of this, but only for a specific numbers of freq.
         *
         * DISCUSS: The implementation should modify the environment of each band freq.
         */
    }
}