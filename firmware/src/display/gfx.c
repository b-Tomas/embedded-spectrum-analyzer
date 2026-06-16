#include "display/gfx.h"

#include "display/SSD1306.h"
#include "display/display.h"
#include "system/eq_mode.h"

#include <stdbool.h>
#include <string.h>

#define PX_PER_BAR ((int)((DISPLAY_WIDTH) / (N_BARS)))

// TODO(b-Tomas): there is code duplication and inconsistencies between the bar charts and the
// EQ UI. Fix in the future

// Bar values in a range of 0-64
uint8_t bars[N_BARS];

// Band values in the range of 0-64
#define PX_PER_BAND ((int)((DISPLAY_WIDTH) / (EQ_BANDS_N)))

void init_bars() {
    // Initialize bars state at 0
    memset(bars, 0, sizeof(bars));
    // Clear the display
    display_clearCanvas();
    display_displayCanvas();
    // calling update_bars() will update the image
}

void update_bars(uint8_t const newBars[N_BARS]) {
    // Repaint only the band between each bar's old and new height
    for (uint8_t barIdx = 0; barIdx < N_BARS; barIdx++) {
        const uint8_t oldVal = bars[barIdx];
        const uint8_t newVal = newBars[barIdx];
        if (newVal == oldVal) {
            continue;
        }

        const uint8_t x1 = barIdx * PX_PER_BAR;
        const uint8_t x2 = (barIdx + 1) * PX_PER_BAR;
        if (newVal > oldVal) {
            display_drawRect(x1, DISPLAY_HEIGHT - newVal, x2, DISPLAY_HEIGHT - oldVal, ON);
        } else {
            display_drawRect(x1, DISPLAY_HEIGHT - oldVal, x2, DISPLAY_HEIGHT - newVal, OFF);
        }
        bars[barIdx] = newVal;
    }

    // Update the display
    display_displayCanvas();
}

void gfx_update_eq_bands(uint8_t const newBands[EQ_BANDS_N], uint8_t selectedBand,
                         bool selectedBandBaseOn) {
    for (uint8_t bandIdx = 0; bandIdx < EQ_BANDS_N; bandIdx++) {
        const uint8_t val = newBands[bandIdx] >> 2; // normalize 0..255 -> 0..63
        const uint8_t x1 = bandIdx * PX_PER_BAND + 1;
        const uint8_t x2 = (bandIdx + 1) * PX_PER_BAND - 1;
        display_drawRect(x1, DISPLAY_HEIGHT - val, x2, DISPLAY_HEIGHT, ON);
        display_drawRect(x1, 0, x2, DISPLAY_HEIGHT - val, OFF);
        if (bandIdx == selectedBand) {
            display_drawRect(x1, DISPLAY_HEIGHT - 4, x2, DISPLAY_HEIGHT, selectedBandBaseOn);
        }
    }

    // Update the display
    display_displayCanvas();
}
