/*
 * SSD1306 128x64 OLED Screen I2C driver
 */

#pragma once

#include <stdint.h>

#define SSD1306_ADDR 0x3C /**< 7-bit I2C address */
#define SSD1306_CMD  0x00 /**< Control byte: command stream. */
#define SSD1306_DATA 0x40 /**< Control byte: data stream. */

#define OLED_WIDTH 128
#define OLED_PAGES 8 /**< 128x64 panel -> 8 pages of 8 rows. */

/* Frame buffer laid out for a single I2C burst: control byte first, then the
 * pixel pages. px[page][col] holds 8 vertical pixels where the LSb is the topmost pixel. */
typedef struct {
    uint8_t ctrl;
    uint8_t px[OLED_PAGES][OLED_WIDTH];
} framebuffer_t;

// Frame buffer the producer draws into. SSD1306_Flush repoints this between two internal
// buffers so a flush can run in the background while the next frame is drawn.
extern framebuffer_t* fb;

/* Command opcodes from the SSD1306 datasheet, grouped as in its command table (Table 9-1).
 * Where a command has two opcodes each opcode gets its own value.
 * Some commands are followed by argument bytes; those are passed to SSD1306_Command,
 * which sends the opcode and its arguments as one command-stream transaction.
 * Each description is quoted verbatim from the datasheet; lines prefixed Note:
 * are driver-specific clarifications.
 * */
typedef enum {
    /* Fundamental commands */

    /* Set Contrast Control.
     * Double byte command to select 1 out of 256 contrast steps. Contrast
     * increases as the value increases. (RESET = 7Fh)
     * Note: one argument byte A[7:0] follows, 0 to 255. */
    SSD1306_SET_CONTRAST = 0x81,
    /* Entire Display ON.
     * A4h, X0=0b: Resume to RAM content display (RESET). Output follows RAM
     * content. */
    SSD1306_OUTPUT_FOLLOWS_RAM = 0xA4,
    /* A5h, X0=1b: Entire display ON. Output ignores RAM content. */
    SSD1306_ENTIRE_DISPLAY_ON = 0xA5,
    /* Set Normal/Inverse Display.
     * A6h, X[0]=0b: Normal display (RESET). 0 in RAM: OFF in display panel.
     * 1 in RAM: ON in display panel. */
    SSD1306_NORMAL_DISPLAY = 0xA6,
    /* A7h, X[0]=1b: Inverse display. 0 in RAM: ON in display panel. 1 in RAM:
     * OFF in display panel. */
    SSD1306_INVERSE_DISPLAY = 0xA7,
    /* Set Display ON/OFF.
     * AEh, X[0]=0b: Display OFF (sleep mode) (RESET). */
    SSD1306_DISPLAY_OFF = 0xAE,
    /* AFh X[0]=1b: Display ON in normal mode. */
    SSD1306_DISPLAY_ON = 0xAF,

    /* Charge pump. Not part of Table 9-1; documented in the SSD1306
     * application note. */

    // Note: one argument byte follows: 0x10 disables the charge pump, 0x14 enables it.
    SSD1306_SET_CHARGE_PUMP = 0x8D,

    /* Scrolling commands */

    /* Continuous Horizontal Scroll Setup.
     * 26h, X[0]=0, Right Horizontal Scroll (horizontal scroll by 1 column).
     * Note: six setup bytes A..F follow: A[7:0] dummy byte (00h), B[2:0] start
     * page, C[2:0] frame interval between scroll steps, D[2:0] end page (must
     * be >= B[2:0]), E[7:0] dummy byte (00h), F[7:0] dummy byte (FFh). */
    SSD1306_RIGHT_HORIZONTAL_SCROLL = 0x26,
    /* 27h, X[0]=1, Left Horizontal Scroll (horizontal scroll by 1 column).
     * Note: same six setup bytes as 0x26. */
    SSD1306_LEFT_HORIZONTAL_SCROLL = 0x27,
    /* Continuous Vertical and Horizontal Scroll Setup.
     * 29h, X1X0=01b: Vertical and Right Horizontal Scroll (horizontal scroll
     * by 1 column).
     * Note: five setup bytes A..E follow: A[7:0] dummy byte, B[2:0] start page,
     * C[2:0] frame interval between scroll steps, D[2:0] end page, E[5:0]
     * vertical scrolling offset in rows. No continuous vertical scrolling is
     * available. */
    SSD1306_VERTICAL_RIGHT_HORIZONTAL_SCROLL = 0x29,
    /* 2Ah, X1X0=10b: Vertical and Left Horizontal Scroll (horizontal scroll
     * by 1 column).
     * Note: same five setup bytes as 0x29. */
    SSD1306_VERTICAL_LEFT_HORIZONTAL_SCROLL = 0x2A,
    /* Deactivate scroll.
     * Stop scrolling that is configured by command 26h/27h/29h/2Ah.
     * Note: after deactivating, the RAM data needs to be rewritten. */
    SSD1306_DEACTIVATE_SCROLL = 0x2E,
    /* Activate scroll.
     * Start scrolling that is configured by the scrolling setup commands:
     * 26h/27h/29h/2Ah. */
    SSD1306_ACTIVATE_SCROLL = 0x2F,
    /* Set Vertical Scroll Area.
     * Note: two argument bytes follow.
     * A[5:0]: Set No. of rows in top fixed area. The No. of rows in top fixed
     * area is referenced to the top of the GDDRAM (i.e. row 0). [RESET = 0]
     * B[6:0]: Set No. of rows in scroll area. This is the number of rows to be
     * used for vertical scrolling. The scroll area starts in the first row
     * below the top fixed area. [RESET = 64] */
    SSD1306_SET_VERTICAL_SCROLL_AREA = 0xA3,

    /* Addressing commands */

    /* Set Lower Column Start Address for Page Addressing Mode (00h~0Fh).
     * Set the lower nibble of the column start address register for Page
     * Addressing Mode using X[3:0] as data bits. The initial display line
     * register is reset to 0000b after RESET.
     * Note: this command is only for page addressing mode. */
    SSD1306_SET_LOWER_COLUMN_START = 0x00,
    /* Set Higher Column Start Address for Page Addressing Mode (10h~1Fh).
     * Set the higher nibble of the column start address register for Page
     * Addressing Mode using X[3:0] as data bits. The initial display line
     * register is reset to 0000b after RESET.
     * Note: this command is only for page addressing mode. */
    SSD1306_SET_HIGHER_COLUMN_START = 0x10,
    /* Set Memory Addressing Mode.
     * Note: one argument byte A[1:0] follows.
     * A[1:0] = 00b, Horizontal Addressing Mode. A[1:0] = 01b, Vertical
     * Addressing Mode. A[1:0] = 10b, Page Addressing Mode (RESET). A[1:0] =
     * 11b, Invalid. */
    SSD1306_SET_MEMORY_ADDRESSING_MODE = 0x20,
    /* Set Column Address.
     * Setup column start and end address. A[6:0]: Column start address,
     * range: 0-127d, (RESET=0d). B[6:0]: Column end address, range: 0-127d,
     * (RESET=127d).
     * Note: two argument bytes follow; only for horizontal or vertical
     * addressing mode. */
    SSD1306_SET_COLUMN_ADDRESS = 0x21,
    /* Set Page Address.
     * Setup page start and end address. A[2:0]: Page start Address, range:
     * 0-7d, (RESET = 0d). B[2:0]: Page end Address, range: 0-7d, (RESET = 7d).
     * Note: two argument bytes follow; only for horizontal or vertical
     * addressing mode. */
    SSD1306_SET_PAGE_ADDRESS = 0x22,
    /* Set Page Start Address for Page Addressing Mode (B0h~B7h).
     * Set GDDRAM Page Start Address (PAGE0~PAGE7) for Page Addressing Mode
     * using X[2:0].
     * Note: this command is only for page addressing mode. */
    SSD1306_SET_PAGE_START = 0xB0,

    /* Hardware configuration */

    /* Set Display Start Line (40h~7Fh).
     * Set display RAM display start line register from 0-63 using
     * X5X4X3X2X1X0. Display start line register is reset to 000000b during
     * RESET. */
    SSD1306_SET_DISPLAY_START_LINE = 0x40,
    /* Set Segment Re-map.
     * A0h, X[0]=0b: column address 0 is mapped to SEG0 (RESET). */
    SSD1306_SEGMENT_REMAP_COL0 = 0xA0,
    /* A1h, X[0]=1b: column address 127 is mapped to SEG0. (Flips the image
     * horizontally.) */
    SSD1306_SEGMENT_REMAP_COL127 = 0xA1,
    /* Set Multiplex Ratio.
     * Note: one argument byte A[5:0] follows.
     * Set MUX ratio to N+1 MUX. N=A[5:0]: from 16MUX to 64MUX, RESET=111111b
     * (i.e. 63d, 64MUX). A[5:0] from 0 to 14 are invalid entry. */
    SSD1306_SET_MULTIPLEX_RATIO = 0xA8,
    /* Set COM Output Scan Direction.
     * C0h, X[3]=0b: normal mode (RESET) Scan from COM0 to COM[N-1]. */
    SSD1306_COM_SCAN_NORMAL = 0xC0,
    /* C8h, X[3]=1b: remapped mode. Scan from COM[N-1] to COM0. Where N is the
     * Multiplex ratio. (Flips the image vertically.) */
    SSD1306_COM_SCAN_REVERSE = 0xC8,
    /* Set Display Offset.
     * Note: one argument byte A[5:0] follows.
     * Set vertical shift by COM from 0d~63d. The value is reset to 00h after
     * RESET. */
    SSD1306_SET_DISPLAY_OFFSET = 0xD3,
    /* Set COM Pins Hardware Configuration.
     * Note: one argument byte follows.
     * A[4]=0b, Sequential COM pin configuration. A[4]=1b (RESET), Alternative
     * COM pin configuration. A[5]=0b (RESET), Disable COM Left/Right remap.
     * A[5]=1b, Enable COM Left/Right remap. */
    SSD1306_SET_COM_PINS = 0xDA,

    /* Timing and driving */

    /* Set Display Clock Divide Ratio/Oscillator Frequency.
     * Note: one argument byte follows.
     * A[3:0]: Define the divide ratio (D) of the display clocks (DCLK):
     * Divide ratio = A[3:0] + 1, RESET is 0000b (divide ratio = 1). A[7:4]:
     * Set the Oscillator Frequency, FOSC. Oscillator Frequency increases with
     * the value of A[7:4] and vice versa. RESET is 1000b. Range: 0000b~1111b.
     * Frequency increases as setting value increases. */
    SSD1306_SET_DISPLAY_CLOCK_DIV = 0xD5,
    /* Set Pre-charge Period.
     * Note: one argument byte follows.
     * A[3:0]: Phase 1 period of up to 15 DCLK clocks; 0 is invalid entry
     * (RESET=2h). A[7:4]: Phase 2 period of up to 15 DCLK clocks; 0 is invalid
     * entry (RESET=2h). */
    SSD1306_SET_PRECHARGE_PERIOD = 0xD9,
    /* Set VCOMH Deselect Level.
     * Note: one argument byte A[6:4] follows.
     * 000b -> 00h -> ~0.65 x VCC. 010b -> 20h -> ~0.77 x VCC (RESET). 011b ->
     * 30h -> ~0.83 x VCC. */
    SSD1306_SET_VCOMH_DESELECT = 0xDB,
    /* NOP. Command for no operation. */
    SSD1306_NOP = 0xE3,
} ssd1306_cmd_t;

/**
 * @brief Initialize the I2C bus and the panel, then turn the display on.
 *
 * Call once at startup, before any SSD1306_Flush.
 */
void SSD1306_Init(void);

/**
 * @brief Send a command as a single command-stream transaction.
 *
 * @param cmd   Command opcode to send.
 * @param args  Argument bytes that follow the opcode, or NULL when the command takes none.
 * @param nargs Number of argument bytes in @p args (0 when there are none).
 */
void SSD1306_Command(ssd1306_cmd_t cmd, const uint8_t* args, uint8_t nargs);

/**
 * @brief Start sending the current framebuffer (fb) to the panel in one I2C transaction.
 *
 * Non-blocking: the transfer runs in the background under interrupt and fb is repointed to a
 * free buffer for the next frame. If the previous flush is still in flight this does nothing,
 * leaving fb unchanged so the producer can keep drawing into it.
 */
void SSD1306_Flush(void);
