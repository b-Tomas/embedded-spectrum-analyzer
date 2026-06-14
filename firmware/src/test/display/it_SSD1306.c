#include "test/display/it_SSD1306.h"

#include "display/SSD1306.h"
#include "display/i2c_bus.h"
#include "test/it.h"

#include <stdint.h>

// Width of the sweeping demo bar in pixels
#define BAR_WIDTH 8

/* Block until any async flush finishes so no frames are dropped. */
static void flush_settle(void) {
    while (i2c_busy()) {
    }
}

/* Sweep a full-height vertical bar across the panel, writing the SSD1306 framebuffer directly and
 * flushing under interrupt. Exercises the raw framebuffer layout and the async flush / i2c_busy
 * handshake. */
static void it_bar_sweep(void) {
    for (uint32_t x = 0; x < OLED_WIDTH; x++) {
        // Full-height vertical bar at column x, wrapping at the right edge.
        for (uint32_t col = 0; col < OLED_WIDTH; col++) {
            uint8_t const on = ((col - x) % OLED_WIDTH) < BAR_WIDTH ? 0xFF : 0x00;
            for (uint32_t page = 0; page < OLED_PAGES; page++) {
                fb->px[page][col] = on;
            }
        }
        SSD1306_Flush();
        it_delay();
        flush_settle();
    }
}

static const it_case_t TESTS[] = {
    {"bar sweep", it_bar_sweep},
};

void it_SSD1306_run(void) {
    it_run_cases("SSD1306", TESTS, IT_ARRAY_LEN(TESTS));
}
