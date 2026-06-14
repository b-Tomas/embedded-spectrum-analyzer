#include "system/nonCFG.h"

#include "lpc17xx.h"
#include "lpc17xx_timer.h"
#include "lpc_types.h"
#include "system/system.h"

const uint32_t TIM1_PRESCALE_1MS = 99;      /**< TC counts every 100 us */
const uint32_t TIM1_MATCH_VALUE_30hz = 333; /**< match every 333 × 100 us = 33.3 ms → 30 Hz */

TIM_TIMERCFG_T tim1_dspl_cfg = {
    .prescaleOpt = TIM_US,
    .prescaleValue = TIM1_PRESCALE_1MS,
};

TIM_MATCHCFG_T tim1_dspl_matchcfg = {
    .channel = TIM_MATCH_0,
    .intEn = ENABLE,
    .stopEn = DISABLE,
    .resetEn = ENABLE,
    .extOpt = TIM_NOTHING,
    .matchValue = TIM1_MATCH_VALUE_30hz,
};

/** After 33.3 ms (30 Hz), set the flag that allows the display to be updated*/
void TIMER1_IRQHandler(void) {

    flag_readyToDisplay = SET;
}