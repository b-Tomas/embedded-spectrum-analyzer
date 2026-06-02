#include "test/display/it_display.h"

#include "display/SSD1306.h"
#include "display/display.h"
#include "test/it.h"

#include <stdint.h>

void it_display_setup(void) {
    display_init();
    display_clearCanvas();
    display_displayCanvas();
}

/* Wipe the whole panel ON column-by-column, then OFF the same way. Exercises the canvas draw path
 * (display_drawPixel / display_displayCanvas) and confirms every pixel is addressable. */
static void it_scan(void) {
    for (uint8_t col = 0; col < DISPLAY_WIDTH; col++) {
        for (uint8_t row = 0; row < DISPLAY_HEIGHT; row++) {
            display_drawPixel(col, row, ON);
        }
        display_displayCanvas();
        it_delay();
    }

    for (uint8_t col = 0; col < DISPLAY_WIDTH; col++) {
        for (uint8_t row = 0; row < DISPLAY_HEIGHT; row++) {
            display_drawPixel(col, row, OFF);
        }
        display_displayCanvas();
        it_delay();
    }
}

static const it_case_t TESTS[] = {
    {"scan: wipe on/off", it_scan},
};

void it_display_run(void) {
    it_run_cases("display", TESTS, IT_ARRAY_LEN(TESTS));
}
