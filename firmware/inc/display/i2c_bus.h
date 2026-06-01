/*
 * Thin wrapper around lpc17xx_i2c.h for use by the display drivers
 */

#pragma once

#include <stdint.h>

#define I2C_DISPlAY_CLOCK_RATE_HZ 400000 /**< Max as per SSD1306 spec */

/**
 * @brief Initializes the I2C bus as consumed by the display drivers
 */
void i2c_init();

/**
 * @brief Transmits a data buffer to a slave device
 *
 * @param buf data buffer to transmit
 * @param len length of the data buffer
 * @param sl_addr7bit slave device address
 */
void i2c_tx(uint8_t* buf, uint32_t len, uint32_t sl_addr7bit);
