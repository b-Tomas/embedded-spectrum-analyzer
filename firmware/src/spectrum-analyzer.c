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


#include "LPC17xx.h"
#include "core_cmInstr.h"
#include "lpc_types.h"
#include "system.h"


#include <cr_section_macros.h>
#include <stdio.h>

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

int main(void) {

    systemInit(realTimeMode);

    while (TRUE) {

        __WFI();
        switch (SYSTEM.mode) {
        case realTimeMode:
            if (!SYSTEM.flag_ModeRunned) {
                configRealTimeMode();
                SYSTEM.flag_ModeRunned = SET;
            }
            // TODO: State implementation.
            break;

        case noiseSamplingMode:
            if (!SYSTEM.flag_ModeRunned) {
                configNoiseSamplingMode();
                SYSTEM.flag_ModeRunned = SET;
            }

            // TODO: State implementation.
            break;

        case equalizerMode:
            if (!SYSTEM.flag_ModeRunned) {
                configEqualizerMode();
                SYSTEM.flag_ModeRunned = SET;
            }

            // TODO: State implementation.
            break;
        }
    }

    return 0;
}
