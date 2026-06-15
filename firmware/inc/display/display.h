/**
 * High-level display manipulation interface. Provides methods for drawing and displaying images.
 * Drawing operations are memory safe; internal frame-buffer bounds are guaranteed.
 *
 * Drawing operations write to a canvas that persists across display() calls.
 *
 * NOTE: for one-shot operations whose canvas **must** be written to the panel because there is no
 * continuous refresh, wait for i2c_busy() before flushing to prevent the frame from being dropped.
 *    ```
 *    display_imageDraw(image);
 *    while (i2c_busy()) { __NOP(); }
 *    display_displayCanvas();
 *    ```
 *
 * TODO(b-Tomas): known possible optimizations:
 *  - transfer changed columns only from display.h to the screen buffer
 *  - i2c transfer by changed page to the display unit
 *  - drop the double buffer, as the canvas is already the back buffer (do care for dropped frames
 *  though, as they are not retried. OK for continuous graphics, bad for still images that don't
 *  call display() on a per-tick basis)
 */

#pragma once

#include "lpc17xx_timer.h"

#include <stdint.h>

#define DISPLAY_WIDTH  (OLED_WIDTH)
#define DISPLAY_HEIGHT (OLED_HEIGHT)

/** Timer prescaler for a 1 ms tick (TC counts every 100 us). */
static const uint32_t TIM1_PRESCALE_1MS = 99;
/** Match value for ≈30 Hz refresh (333 × 100 us = 33.3 ms). */
static const uint32_t TIM1_MATCH_VALUE_30hz = 333;

/**
 * Possible values of a pixel
 */
typedef enum { OFF = 0, ON = !OFF } PixelValue;

/**
 * An image to draw as an unpacked 1px per byte matrix
 */
typedef struct {
    PixelValue* data;
    uint8_t w, h, x, y;
} Image;

/** Timer configuration for the display refresh timer. */
extern TIM_TIMERCFG_T tim1_dspl_cfg;
/** Match configuration for the display refresh timer. */
extern TIM_MATCHCFG_T tim1_dspl_matchcfg;

/**
 * @brief initialize the display interface and the timer 1 to refresh it.
 */
void display_init();

/**
 * @brief write the canvas to the screen. Non-blocking. Drops the frame if busy.
 */
void display_displayCanvas();

/**
 * @brief clear the image buffer
 */
void display_clearCanvas();

/**
 * @brief draw an individual pixel state onto the canvas
 */
void display_drawPixel(uint8_t x, uint8_t y, PixelValue state);

/**
 * @brief draw an image on the canvas
 */
void display_imageDraw(Image image);

/**
 * @brief turn ON those pixels that map to 1's on the canvas
 */
void display_imageSet(Image setMask);

/**
 * @brief turn OFF those pixels that map to 1's on the canvas
 */
void display_imageClear(Image clearMask);

/**
 * @brief toggle the pixel state of pixels that map to 1's on the canvas
 */
void display_imageInvert(Image invertMask);

/**
 * @brief Draw a rectangle on the canvas
 *
 * @param x1 top-left horizontal coordinate of the rectangle
 * @param y1 top-left vertical coordinate of the rectangle
 * @param x2 top-right horizontal coordinate of the rectangle
 * @param y2 top-right vertical coordinate of the rectangle
 * @param value value to give the pixels within the rectangle
 */
void display_drawRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, PixelValue value);
