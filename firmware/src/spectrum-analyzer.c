#include "test/it.h"

#include <stdbool.h>

int main(void) {
    // TODO: real application entry point. For now this build runs the display
    // integration test suite on a loop (see src/test/it_display.c).
    while (true) {
        it_run_all();
    }

    return 0;
}
