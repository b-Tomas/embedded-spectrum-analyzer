#pragma once

#include "dsp/dsp.h"
#include "lpc_types.h"

#include <stddef.h>

#define N_SAMPLES      8
#define COOLDOWN_TICKS 10

/**
 * @brief Available operation modes for the system.
 */
typedef enum {
    realTimeMode,
    noiseSamplingMode,
    equalizerMode,
} Mode;

extern FlagStatus flag_readyToDisplay;
extern FlagStatus flag_SamplingCooldownReady;

/**
 * @brief The orchestrator struct.
 * @details Contains the state variables.
 * @note flag_ModeConfigured is used in the state machine.
 */
typedef struct {
    Mode mode;                      /**< Current operational mode. */
    FlagStatus flag_ModeConfigured; /**< Flag to prevent re-configuration. */
    Filter filter;                  /**< Active signal filter. */
} System;

/**
 * @brief Global orchestrator instance.
 */
extern System SYSTEM;

/**
 * @brief System initialisation.
 * @param mode The mode in which the system starts.
 */
void system_Init(Mode mode);

/**
 * @brief System configuration for real-time mode.
 * @details
 * - Configures the peripherals whose behavior needs to change for this mode.
 * - Applies the appropriate filter.
 */
void system_ConfigureSetting_RealTimeMode(void);

/**
 * @brief System configuration for noise sampling mode.
 * @details Configures the peripherals whose behavior needs to change for this mode.
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
 * @brief Real-time mode tick.
 * @details Processes the signal and transfers it to the peripherals via DMA.
 */
void system_tickRealTimeMode(void);

/**
 * @brief Noise sampling mode tick.
 * @details Processes the signal and saves it for use as a noise-suppression filter.
 */
void system_tickNoiseSamplingMode(void);

/**
 * @brief The implementation of the equalizer mode.
 * @details UI experience to configure the gains bands
 * @note Must update EQUALIZER.
 */
void system_tickEqualizerMode(void);

/* ==========================================================================
 * Getters and Setters
 * ========================================================================== */

/**
 * @brief Sets a new operational mode.
 * @param mode The new mode.
 */
void system_setMode(Mode mode);