/*
 * Copyright 2022 NXP
 * NXP confidential.
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#endif

#include <cr_section_macros.h>
#include <stdio.h>

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

int main(void) {
    // TODO: real application entry point. For now this build runs the display
    // integration test suite on a loop (see src/test/it_display.c).
    while (true) {
        it_run_all();
    }

    return 0;
}
