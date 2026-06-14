#pragma once

#include "lpc_types.h"

/**
 * @brief Available operation modes for the system.
 */
typedef enum {
    realTimeMode,
    noiseSamplingMode,
    equalizerMode,
} Mode;

/**
 * @brief Possible filters that can be applied to modify the signal.
 */
typedef enum {
    passthrough,
    noiseSuppression,
    customEqualized,
} Filter;

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
void systemInit(Mode mode);

/**
 * @brief System config for real time mode.
 * @details
 * - Configures the peripherals whose behaivor needs to be changed due to the new mode.
 * - Applies the appropriate filter.
 */
void configRealTimeMode(void);

/**
 * @brief System config for noise sampling mode.
 * @details Configures the peripherals whose behaivor needs to be changed due to the new mode.
 */
void configNoiseSamplingMode(void);

/**
 * @brief System config for equalizer mode.
 * @details
 * - Configures the peripherals whose behavior needs to be changed due to the new mode.
 * - Disable the triggers for other modes.
 */
void configEqualizerMode(void);

/**
 * @brief The implementation of the real time mode.
 * @details Process the signal and transfer it to the peripherals by DMA
 */
void executeRealTimeMode(void);

/**
 * @brief The implementation of the noise sampling mode.
 * @details Process the signal and saves it for use as noise-suppression filter
 */
void executeNoiseSamplingMode(void);

/**
 * @brief The implementation of the equalizer mode.
 * @details UI experience to configure the gains bands
 * @note Must update EQUALIZER.
 */
void executeEqualizerMode(void);

/**
 * @brief handle a keypress depending on the current mode
 * @param sym the symbol to process
 */
void handleKey(uint8_t sym);

//=================================================================
// Getters y Setters
//=================================================================

/**
 * @brief Sets a new operational mode.
 * @param mode the new mode.
 */
void setMode(Mode mode);

/**
 * @brief Selects one of the filters.
 * @param filter The new filter to be applied.
 */
void setFilter(Filter filter);

/**
 * @brief Clear the filter.
 */
void clearFilter(void);
