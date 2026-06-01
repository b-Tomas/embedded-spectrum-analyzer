#include "display/i2c_bus.h"

#include "lpc17xx_i2c.h"
#include "lpc17xx_pinsel.h"

void i2c_init() {
    // P0.27 (SDA0) and P0.28 (SCL0) are the dedicated I2C0 pins.
    // Function 01 switches them onto open-drain mode. The bus relies on external pull-ups
    // R57/R58 3.3 kohm provided by the LPCXpresso 1769 board.
    PINSEL_CFG_T pin = {
        .port = PORT_0, .func = PINSEL_FUNC_01, .mode = PINSEL_TRISTATE, .openDrain = DISABLE};
    pin.pin = PIN_27; // SDA0
    PINSEL_ConfigPin(&pin);
    pin.pin = PIN_28; // SCL0
    PINSEL_ConfigPin(&pin);

    // Standard-drive I2C (no NXP Fast-Mode) with the glitch filter and slew-rate control
    // enabled, which covers bus clocks up to 400 kHz.
    PINSEL_SetI2CPins(PINSEL_I2C_NORMAL, ENABLE);

    I2C_Init(LPC_I2C0, I2C_DISPlAY_CLOCK_RATE_HZ);
    I2C_Cmd(LPC_I2C0, ENABLE);
}

/* Polling (blocking) master transmit; retries up to 3 times on bus errors. */
void i2c_tx_sync(uint8_t* buf, const uint32_t len, const uint32_t sl_addr7bit) {
    I2C_M_SETUP_Type t = {0};
    t.sl_addr7bit = sl_addr7bit;
    t.tx_data = buf;
    t.tx_length = len;
    t.retransmissions_max = 3;
    I2C_MasterTransferData(LPC_I2C0, &t, I2C_TRANSFER_POLLING);
}

/* Configuration for the async transfer. I2C_MasterTransferData keeps a pointer to it and
 * drives the bytes out from the ISR, so it must outlive the i2c_tx_async call.
 * i2c_busy reads its status field. */
static I2C_M_SETUP_Type async_tx;

/* Interrupt-driven master transmit; returns immediately. I2C_MasterTransferData enables the
 * I2C0 interrupt in the NVIC, which i2c_irq_handler then services. Retries up to 3 times on bus
 * errors. */
void i2c_tx_async(uint8_t* buf, const uint32_t len, const uint32_t sl_addr7bit) {
    async_tx = (I2C_M_SETUP_Type){0};
    async_tx.sl_addr7bit = sl_addr7bit;
    async_tx.tx_data = buf;
    async_tx.tx_length = len;
    async_tx.retransmissions_max = 3;
    I2C_MasterTransferData(LPC_I2C0, &async_tx, I2C_TRANSFER_INTERRUPT);
}

/* Read I2C transfer completion. tx_data is NULL before the first transfer, when the bus is idle. */
bool i2c_busy(void) {
    return async_tx.tx_data != NULL && !(async_tx.status & I2C_SETUP_STATUS_DONE);
}

/* Handle display-related I2C interrupts. Call this from within the I2C ISR. */
void i2c_irq_handler(void) {
    I2C_MasterHandler(LPC_I2C0);
}
