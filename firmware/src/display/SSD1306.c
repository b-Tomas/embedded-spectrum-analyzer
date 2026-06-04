#include "display/SSD1306.h"

#include "display/i2c_bus.h"

#include <stddef.h>
#include <stdint.h>

/* Double framebuffers to safely draw the next frame while SSD1306_Flush ships the
 * current one in the background. */
static framebuffer_t fb_a, fb_b;

/* Pointer to the framebuffer the producer draws into. */
framebuffer_t* fb = &fb_a;

/* Command order and arguments follow the "Internal setting (Charge pump)" I2C init code in
 * docs/SSD1306/UG-2864HSWEG01 user guide.pdf (Section 7.2, p19).
 * The guide is a reasonable reference for driving the panel though it is not the exact same board
 * we have.
 *
 * Deviation: this driver selects horizontal addressing (20h) and locks the window to the whole
 * panel (21h/22h, set after Display ON) so SSD1306_Flush ships the framebuffer in one auto-wrapping
 * burst rather than page by page. */
void SSD1306_Init() {
    i2c_init();

    SSD1306_Command(SSD1306_DISPLAY_OFF, NULL, 0);
    // Deviation from user guide: horizontal addressing.
    SSD1306_Command(SSD1306_SET_MEMORY_ADDRESSING_MODE, (uint8_t[]){0x00}, 1);
    SSD1306_Command(SSD1306_SET_DISPLAY_START_LINE, NULL, 0); // line 0
    SSD1306_Command(SSD1306_SET_CONTRAST, (uint8_t[]){0xCF}, 1);
    SSD1306_Command(SSD1306_SEGMENT_REMAP_COL127, NULL, 0);
    // Flip the image vertically. Origin of the pixel coordinates is the top-left corner
    SSD1306_Command(SSD1306_COM_SCAN_REVERSE, NULL, 0);
    SSD1306_Command(SSD1306_NORMAL_DISPLAY, NULL, 0);
    SSD1306_Command(SSD1306_SET_MULTIPLEX_RATIO, (uint8_t[]){0x3F},
                    1); // 1/64 duty, all 64 rows active
    SSD1306_Command(SSD1306_SET_DISPLAY_OFFSET, (uint8_t[]){0x00}, 1); // no vertical shift
    SSD1306_Command(SSD1306_SET_DISPLAY_CLOCK_DIV, (uint8_t[]){0x80}, 1);
    SSD1306_Command(SSD1306_SET_PRECHARGE_PERIOD, (uint8_t[]){0xF1}, 1);
    SSD1306_Command(SSD1306_SET_COM_PINS, (uint8_t[]){0x12},
                    1); // alternative COM pin layout, for 128x64
    SSD1306_Command(SSD1306_SET_VCOMH_DESELECT, (uint8_t[]){0x40}, 1);
    SSD1306_Command(SSD1306_SET_CHARGE_PUMP, (uint8_t[]){0x14},
                    1); // enable the internal charge pump
    SSD1306_Command(SSD1306_DISPLAY_ON, NULL, 0);

    // Deviation from user guide: Lock the addressing window to the whole panel once.
    // In horizontal mode the RAM pointer auto-wraps after 1024 bytes, so every frame is a single
    // pure-data burst with no per-frame command overhead.
    SSD1306_Command(SSD1306_SET_COLUMN_ADDRESS, (uint8_t[]){0x00, 0x7F}, 2); // columns 0..127
    SSD1306_Command(SSD1306_SET_PAGE_ADDRESS, (uint8_t[]){0x00, 0x07}, 2);   // pages 0..7

    fb_a.ctrl = SSD1306_DATA; // control byte, set once per buffer
    fb_b.ctrl = SSD1306_DATA;
}

/* Build the control byte, the opcode, and its argument bytes, and send them as one command-stream
 * transaction (datasheet Figure 8-7). The longest possible command (horizontal scroll setup)
 * takes six argument bytes. */
void SSD1306_Command(ssd1306_cmd_t cmd, const uint8_t* args, uint8_t nargs) {
    uint8_t buf[8]; // control byte + opcode + up to 6 argument bytes
    buf[0] = SSD1306_CMD;
    buf[1] = cmd;
    for (uint8_t i = 0; i < nargs; i++) {
        buf[2 + i] = args[i];
    }
    i2c_tx_sync(buf, 2 + nargs, SSD1306_ADDR);
}

/* Each framebuffer starts with the data control byte and is stored contiguously (see
 * framebuffer_t), so the whole struct goes out as a single I2C buffer.
 *
 * The flush hands the just-drawn buffer to the background transfer and flips fb to the other
 * buffer for the next frame. If the previous transfer is still running it does nothing and the
 * producer keeps drawing into the same buffer. A buffer only returns to the producer once its
 * transfer has finished, so the producer never writes a buffer that is still being sent. */
void SSD1306_Flush(void) {
    if (i2c_busy()) {
        return;
    }
    framebuffer_t* sent = fb;
    fb = (fb == &fb_a) ? &fb_b : &fb_a;
    i2c_tx_async((uint8_t*)sent, sizeof(*sent), SSD1306_ADDR);
}
