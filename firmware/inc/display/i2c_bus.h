/*
 * Thin wrapper around lpc17xx_i2c.h for use by the display drivers
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#define I2C_DISPlAY_CLOCK_RATE_HZ 400000 /**< Max as per SSD1306 spec */

/**
 * @brief Initializes the I2C bus as consumed by the display drivers
 */
void i2c_init();

/**
 * @brief Transmits a data buffer to a slave device, blocking until the transfer completes.
 *
 * @param buf data buffer to transmit
 * @param len length of the data buffer
 * @param sl_addr7bit slave device address
 */
void i2c_tx_sync(uint8_t* buf, uint32_t len, uint32_t sl_addr7bit);

/**
 * @brief Starts transmitting a data buffer to a slave device and returns immediately.
 *
 * The transfer runs in the background under interrupt. @p buf must stay valid until the
 * transfer completes (see i2c_busy). Only one async transfer may be in flight at a time.
 *
 * @param buf data buffer to transmit
 * @param len length of the data buffer
 * @param sl_addr7bit slave device address
 */
void i2c_tx_async(uint8_t* buf, uint32_t len, uint32_t sl_addr7bit);

/**
 * @brief Reports whether the last i2c_tx_async transfer is still in flight.
 *
 * @return true while an async transfer is in progress, false once it has finished
 */
bool i2c_busy(void);

/**
 * @brief Drives the in-flight async transfer one step forward.
 *
 * The application must call this from its I2C0_IRQHandler so the bus owns its interrupt
 * vector through the application's vector table.
 */
void i2c_irq_handler(void);
