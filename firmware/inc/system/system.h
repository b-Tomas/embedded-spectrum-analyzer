#pragma once

#include "dsp/dsp.h"
#include "lpc_types.h"

#include <stddef.h>

/**
 * @brief Available operation modes for the system.
 */
typedef enum {
    realTimeMode,
    noiseSamplingMode,
    equalizerMode,
} Mode;

extern FlagStatus flag_readyToDisplay;

/**
 * @brief The orchestrator struct.
 * @details Contains de state variabes.
 * @note flag_ModeExecuted is used in the sate machine.
 */
typedef struct {
    Mode mode;                      /**< Current operational mode. */
    FlagStatus flag_ModeConfigured; /**< Flag to prevent re-configuration */
    Filter filter;                  /**< Active signal filter. */
} System;

/**
 * @brief Global orchestrator instance.
 */
extern System SYSTEM;

/**
 * @brief System init.
 * @param mode The mode in which system inits.
 */
void system_Init(Mode mode);

/**
 * @brief System config for real time mode.
 * @details
 * - Configures the peripherals whose behaivor needs to be changed due to the new mode.
 * - Applies the appropriate filter.
 */
void system_ConfigureSetting_RealTimeMode(void);

/**
 * @brief System config for noise sampling mode.
 * @details Configures the peripherals whose behaivor needs to be changed due to the new mode.
 */
void system_ConfigureSetting_NoiseSamplingMode(void);

/**
 * @brief System config for equalizer mode.
 * @details
 * - Configures the peripherals whose behavior needs to be changed due to the new mode.
 * - Disable the triggers for other modes.
 */
void system_ConfigureSetting_EqualizerMode(void);

/**
 * @brief The implementation of the real time mode.
 * @details Process the signal and transfer it to the peripherals by DMA
 */
void system_executeRealTimeMode(void);

/**
 * @brief The implementation of the noise sampling mode.
 * @details Process the signal and saves it for use as noise-suppression filter
 */
void system_StartNoiseSamplingMode(void);

/**
 * @brief The implementation of the equalizer mode.
 * @details UI experience to configure the gains bands
 * @note Must update EQUALIZER.
 */
void system_StartEqualizerMode(void);

//=================================================================
// Getters y Setters
//=================================================================

/**
 * @brief Sets a new operational mode.
 * @param mode the new mode.
 */
void system_setMode(Mode mode);
