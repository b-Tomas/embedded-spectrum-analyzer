/**
 * Keyboard handling logic. Stores keypresses on keyboard interrupt to a buffer that drops
 * keypresses when full.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

/** @brief Ring buffer slot count. Usable capacity is KBD_BUFFER_SIZE - 1. Must be a power of 2. */
#define KBD_BUFFER_SIZE 4

/**
 * @brief Initialize keyboard I/O and enable keyboard interrupts
 */
void kbd_init(void);

/**
 * @brief reads the symbol from GPIO on keypress and stores it in the buffer
 * Must be called from EINT0_IRQHandler
 */
void kbd_irq_handler(void);

/**
 * @brief Stores a symbol in the ring buffer, dropping it if the buffer is full.
 *
 * @param sym the symbol to store
 */
void kbd_push(uint8_t sym);

/**
 * @brief returns the oldest item in the buffer if any
 *
 * @param sym the key to process if any
 * @return whether key was returned
 */
bool kbd_pop(uint8_t* sym);
