#pragma once

#include "dsp/dsp.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"
#include "lpc_types.h"

/**
 * ------------------------------------------------------------------------------------
 *                     Display non-configurable settings
 * ------------------------------------------------------------------------------------
 */

/**<             TIMER1 configuration                    */

extern const uint32_t TIM1_PRESCALE_FOR_15HZ;
/**
 * @brief prescaler option and prescaler value for timer 1
 */
extern TIM_TIMERCFG_T tim1_dspl_cfg;

/**
 * @brief match configuration for timer 1
 *
 */
extern TIM_MATCHCFG_T tim1_dspl_matchcfg;