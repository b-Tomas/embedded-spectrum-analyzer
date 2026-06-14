#include "test/display/it_gfx.h"

#include "display/gfx.h"
#include "test/it.h"

#include <string.h>

void it_rotatingBars() {
    init_bars();
    uint8_t bars[10] = {10, 50, 32, 64, 20, 43, 9, 2, 60, 22};
    for (int j = 0; j < sizeof(bars) * 3; j++) {
        // while (1) {
        update_bars(bars);
        for (int i = 0; i < 20; i++) {
            it_delay();
        }
        const uint8_t barTmp = bars[0];
        memcpy(bars, bars + 1, sizeof(bars) - 1);
        bars[sizeof(bars) - 1] = barTmp;
    }
}

static const it_case_t TESTS[] = {
    {"bars: rotate", it_rotatingBars},
};

void it_gfx_bars_run(void) {
    it_run_cases("gfx", TESTS, IT_ARRAY_LEN(TESTS));
}
