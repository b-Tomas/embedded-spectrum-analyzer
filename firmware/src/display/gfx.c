#include "display/gfx.h"

#include "display/SSD1306.h"
#include "display/display.h"

#include <string.h>

#define PX_PER_BAR ((int)((DISPLAY_WIDTH) / (N_BARS)))

// Bar values in a range of 0-64
uint8_t bars[N_BARS];

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
