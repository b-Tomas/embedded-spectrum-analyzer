#include "display/display.h"
#include "display/i2c_bus.h"

#include <stdint.h>

/* The display bus leaves its interrupt vector to the application. Route it to the driver so
 * SSD1306_Flush can transmit in the background. */
void I2C0_IRQHandler(void) {
    i2c_irq_handler();
}

/* --------------------------------- setup ---------------------------------- */
void setup(void) {
    display_init();
    display_clearCanvas();
    display_displayCanvas();
}

#define BAR_WIDTH 8 /**< Width of the sweeping demo bar, in pixels. */

void delay() {
    for (volatile int i = 0; i < 100000; i++) {
        __NOP();
    }
}

int main(void) {
    setup();

    // uint32_t x = 0;
    // while (1) {
    //     // Full-height vertical bar at column x, wrapping at the right edge.
    //     for (uint32_t col = 0; col < OLED_WIDTH; col++) {
    //         uint8_t on = ((col - x) % OLED_WIDTH) < BAR_WIDTH ? 0xFF : 0x00;
    //         for (uint32_t page = 0; page < OLED_PAGES; page++) {
    //             fb->px[page][col] = on;
    //         }
    //     }
    //     SSD1306_Flush();
    //     if (++x >= OLED_WIDTH) {
    //         x = 0;
    //     }
    //     while (i2c_busy()) {
    //     }
    // }

    while (true) {
        for (uint8_t col = 0; col < DISPLAY_WIDTH; col++) {
            for (uint8_t row = 0; row < DISPLAY_HEIGHT; row++) {
                display_drawPixel(col, row, ON);
            }
            display_displayCanvas();
            delay();
        }

        for (uint8_t col = 0; col < DISPLAY_WIDTH; col++) {
            for (uint8_t row = 0; row < DISPLAY_HEIGHT; row++) {
                display_drawPixel(col, row, OFF);
            }
            display_displayCanvas();
            delay();
        }
    }

    return 0;
}
