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
void i2c_tx(uint8_t* buf, const uint32_t len, const uint32_t sl_addr7bit) {
    I2C_M_SETUP_Type t = {0};
    t.sl_addr7bit = sl_addr7bit;
    t.tx_data = buf;
    t.tx_length = len;
    t.retransmissions_max = 3;
    I2C_MasterTransferData(LPC_I2C0, &t, I2C_TRANSFER_POLLING);
}
