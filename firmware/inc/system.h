#pragma once

#include "lpc_types.h"

#define CRUDE_SIGNAL "Memory Address where the signal is loaded" // Memory Address where the signal is loaded
#define PROCECED_SIGNAL "Memory Address where the signal is loaded after the filter" // Memory Address where the signal is loaded after the filter
#define MAX_BAND 10 // Number of equalizer bands. 

typedef enum {
    realTimeMode,
    noiseSamplingMode,
    equalizerMode,
} Mode;

typedef struct {
    Mode mode;
    Mode previousMode;
    FlagStatus flag_ModeRunned;

} System;

/* @brief The orchestrator
 */
extern System SYSTEM;

typedef enum {
    passthrogh,
    noiseSupression,
    customEqualized,
} Filer;

/*  @brief the values of every band.
 * used on
 */
extern uint32_t EQUALIZER[MAX_BAND];

/*@brief System init
 * @param mode The mode in which system inits.
 */
void systemInit(Mode mode);

/* @brief System config for real time mode.
 * - Config the peripherals whose beheivor needs to changed due to the new mode.
 * - Applies the appropriate filter
 */
void configRealTimeMode(void);

/* @brief System config for noise sampling mode.
 * - Config the peripherals whose beheivor needs to changed due to the new mode.
 */
void configNoiseSamplingMode(void);

/* @brief System config for equalizer mode.
 * - Config the peripherals whose beheivor needs to changed due to the new mode.
 * - Disable the triggers for other modes.
 */
void configEqualizerMode(void);

///* @brief Función que ejecuta a la señal obtenida el filtro seleccionado.
// *
// */
// void executeFiler();

/* @brief rewrite EQUALIZER.
 * @param newBrands MAX_BRAND size array that cointains the new EQ values.
 */
void changeEqualizer(const uint32_t* newBrands);

//=================================================================
// Getters y Setters
//=================================================================

/* @brief Set a new mode.
 * @param mode the new mode.
 */
void setMode(Mode mode);

/* @brief Get the previous mode.
 * @ return Mode the previous mode.
 */
Mode getPreviousMode();

/* @brief set the before mode value into the current.
 */
void setPreviousMode(void);

/* @brief Select one of the filters,
 * @param filter The new filter.
 */
void setFilter(Filer filter);
 
