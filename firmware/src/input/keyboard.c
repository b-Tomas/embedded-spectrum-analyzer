#include "input/keyboard.h"

#include "lpc17xx_exti.h"
#include "lpc17xx_gpio.h"

#include <stdbool.h>

// Ring buffer to store keypresses
typedef struct {
    uint8_t data[KBD_BUFFER_SIZE]; // FIFO ring buffer
    uint8_t head;                  // where the ISR will write next
    uint8_t tail;                  // oldest unread item
} KBD_BUF_T;

static volatile KBD_BUF_T kbd_buf = {0};

void kbd_init() {
    // Configure and enable interruputs for the DA (data available) line of the keyboard decoder
    EXTI_Init();
    EXTI_PinConfig(EXTI_EINT0, EXTI_PULLDOWN);
    EXTI_ConfigEnable(&(EXTI_CFG_T){EXTI_EINT0, EXTI_EDGE_SENSITIVE, EXTI_RISING_EDGE});
    // Configure GPIO input for the 4 data lines
    // Use the lower 4 bits of port 2
    GPIO_SetDir(PORT_2, 0x0F, GPIO_INPUT);
}

void kbd_push(uint8_t const sym) {
    uint8_t const next = (kbd_buf.head + 1) & (KBD_BUFFER_SIZE - 1);
    if (next == kbd_buf.tail)
        return; // buffer is full
    // Store the symbol in the buffer and advance the head pointer
    kbd_buf.data[kbd_buf.head] = sym;
    kbd_buf.head = next;
}

void kbd_irq_handler() {
    kbd_push(GPIO_ReadValue(PORT_2) & 0x0F);
}

bool kbd_pop(uint8_t* sym) {
    if (kbd_buf.head == kbd_buf.tail)
        return false; // buffer is empty
    // Return the tail of the ring buffer
    *sym = kbd_buf.data[kbd_buf.tail];
    kbd_buf.tail = (kbd_buf.tail + 1) & (KBD_BUFFER_SIZE - 1);
    return true;
}
