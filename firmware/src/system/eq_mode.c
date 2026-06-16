#include "system/eq_mode.h"

#include "display/gfx.h"
#include "lpc_types.h"
#include "system/system.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define EQ_INC_DEC 16 /**< How much to increment or decrement each bar **/

/**< User-configured values of EQ bands in a 0-255 scale **/
uint8_t eq_bands[EQ_BANDS_N] = {255, 255, 255, 255, 255, 255, 255, 255};

/**< Currently selected band in the range of EQ_BANDS. Signed to identify overflow/underflow **/
static int8_t curr_band;
/**< Temporary EQ to be saved to eq_bands **/
static uint8_t tmp_eq_bands[sizeof(eq_bands)];

/**< Indicates whether the base of the selected band is on or off for a blinking effect **/
static bool selectedBandBlink = false;
#define EQ_BANDS_BLINK_TICKS 15
static uint8_t selectedBandBlinkCounter = 0;

void system_ConfigureSetting_EqualizerMode(void) {
    curr_band = 0;
    memcpy(tmp_eq_bands, eq_bands, sizeof(eq_bands));
}

void system_tickEqualizerMode(void) {
    if (flag_readyToDisplay) {
        flag_readyToDisplay = RESET;

        selectedBandBlinkCounter = (selectedBandBlinkCounter + 1) % EQ_BANDS_BLINK_TICKS;
        if (selectedBandBlinkCounter == 0) {
            selectedBandBlink = !selectedBandBlink;
        }
        gfx_update_eq_bands(tmp_eq_bands, curr_band, selectedBandBlink);
    }
}

void system_processKeyEqualizerMode(char const key) {
    switch (key) {
    case '2':
        // Increase the value of the current band
        tmp_eq_bands[curr_band] = MIN(255, tmp_eq_bands[curr_band] + EQ_INC_DEC);
        break;
    case '8':
        // Decrease the value of the current band
        tmp_eq_bands[curr_band] = MAX(0, tmp_eq_bands[curr_band] - EQ_INC_DEC);
        break;
    case '4':
        // Select the band to the left
        curr_band = MAX(curr_band - 1, 0);
        selectedBandBlinkCounter = 0;
        selectedBandBlink = false;
        break;
    case '6':
        // Select the band to the right
        curr_band = MIN(curr_band + 1, EQ_BANDS_N);
        selectedBandBlinkCounter = 0;
        selectedBandBlink = false;
        break;
    case '#':
        memcpy(eq_bands, tmp_eq_bands, sizeof(eq_bands));
        printf("system_processKeyEqualizerMode: saved EQ ");
        for (int i = 0; i < sizeof(eq_bands); i++) {
            printf("%d ", eq_bands[i]);
        }
        printf("\n");
        break;
    default:
        printf("system_processKeyEqualizerMode: key=%c unrecognized\n", key);
    }
}
