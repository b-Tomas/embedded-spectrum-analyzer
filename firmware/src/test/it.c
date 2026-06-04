#include "test/it.h"

#include "LPC17xx.h"
#include "test/display/it_SSD1306.h"
#include "test/display/it_display.h"
#include "test/display/it_gfx.h"

#include <stdio.h>

void it_delay() {
    for (volatile int i = 0; i < 100000; i++) {
        __NOP();
    }
}

void it_run_cases(const char* group, const it_case_t* cases, uint32_t const n) {
    for (uint32_t i = 0; i < n; i++) {
        printf("[IT %s %u/%lu] %s\n", group, (uint32_t)(i + 1), (unsigned long)n, cases[i].name);
        cases[i].run();
    }
}

void it_run_all(void) {
    /* Display subsystem. */
    it_display_setup();
    it_SSD1306_run();
    it_display_run();

    /* Graphics engine */
    it_gfx_bars_run();

    /* TODO(b-Tomas): Add other subsystems as they are implemented. */

    printf("[IT] done\n");
}
