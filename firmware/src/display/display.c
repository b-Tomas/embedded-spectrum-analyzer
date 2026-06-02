#include "display/display.h"

#include "display/SSD1306.h"

#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/**
 * The 128x64 persistent canvas this module draws onto. It has the same format (packed) as
 * the `fb->px` (data) field for easy memcpy on each @ref display_displayCanvas call,
 * efficient for ticked engines.
 * Drawing has the overhead of translating from unpacked @ref Image data into the packed format
 *
 * Refer to @ref framebufferData_t to understand the pixel addressing
 */
static framebufferData_t canvas;

/* packed byte holding pixel (x,y) */
#define CANVAS_BYTE(x, y) (canvas[(y) >> 3][(x)])
/* the bit corresponding to a pixel bit within a byte */
#define CANVAS_MASK(y) (1U << ((y) & 0b111))

void display_init() {
    SSD1306_Init();
}

void display_displayCanvas() {
    memcpy(fb->px, canvas, sizeof(canvas));
    SSD1306_Flush();
}

void display_clearCanvas() {
    memset(canvas, 0, sizeof(canvas));
}

void display_drawPixel(uint8_t const x, uint8_t const y, PixelValue const state) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT)
        return;

    if (state != OFF) {
        CANVAS_BYTE(x, y) |= CANVAS_MASK(y);
    } else {
        CANVAS_BYTE(x, y) &= ~CANVAS_MASK(y);
    }
}

void display_imageDraw(Image const image) {
    for (uint8_t y = image.y; y < (uint8_t)MIN(image.h + image.y, DISPLAY_HEIGHT); y++) {
        for (uint8_t x = image.x; x < (uint8_t)MIN(image.w + image.x, DISPLAY_WIDTH); x++) {
            display_drawPixel(x, y, image.data[y * image.w + x]);
        }
    }
}

void display_imageSet(Image const setMask) {
    for (uint8_t y = setMask.y; y < (uint8_t)MIN(setMask.h + setMask.y, DISPLAY_HEIGHT); y++) {
        for (uint8_t x = setMask.x; x < (uint8_t)MIN(setMask.w + setMask.x, DISPLAY_WIDTH); x++) {
            if (setMask.data[(y - setMask.y) * setMask.w + (x - setMask.x)] != OFF) {
                CANVAS_BYTE(x, y) |= CANVAS_MASK(y);
            }
        }
    }
}

void display_imageClear(Image const clearMask) {
    for (uint8_t y = clearMask.y; y < (uint8_t)MIN(clearMask.h + clearMask.y, DISPLAY_HEIGHT);
         y++) {
        for (uint8_t x = clearMask.x; x < (uint8_t)MIN(clearMask.w + clearMask.x, DISPLAY_WIDTH);
             x++) {
            if (clearMask.data[(y - clearMask.y) * clearMask.w + (x - clearMask.x)] != OFF) {
                CANVAS_BYTE(x, y) &= ~CANVAS_MASK(y);
            }
        }
    }
}

void display_imageInvert(Image const invertMask) {
    for (uint8_t y = invertMask.y; y < (uint8_t)MIN(invertMask.h + invertMask.y, DISPLAY_HEIGHT);
         y++) {
        for (uint8_t x = invertMask.x; x < (uint8_t)MIN(invertMask.w + invertMask.x, DISPLAY_WIDTH);
             x++) {
            if (invertMask.data[(y - invertMask.y) * invertMask.w + (x - invertMask.x)] != OFF) {
                CANVAS_BYTE(x, y) ^= CANVAS_MASK(y);
            }
        }
    }
}
