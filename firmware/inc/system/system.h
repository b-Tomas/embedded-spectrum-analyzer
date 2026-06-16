#pragma once

#include "dsp/dsp.h"
#include "lpc_types.h"

/**
 * @brief Available operation modes for the system.
 */
typedef enum {
    realTimeMode,
    noiseSamplingMode,
    equalizerMode,
    _modeCount,
} Mode;

/**
 * @brief all methods a mode definition must expose
 */
typedef struct {
    void (*init)(void);        /**< Initialize them mode **/
    void (*deInit)(void);      /**< De-initialize them mode **/
    void (*tick)(void);        /**< Implementation of the mode **/
    void (*handleKey)(char c); /**< Process a key in the context of the mode **/
} SystemMode_T;

/**< Registered hooks **/
extern SystemMode_T MODES[_modeCount];

/**
 * @brief for each mode to register its implementation
 */
void system_registerMode(Mode mode, SystemMode_T const* hooks);
void realTimeMode_registerHooks(void);
void noiseSamplingMode_registerHooks(void);
void eqMode_registerHooks(void);

/**
 * @brief The orchestrator struct.
 * @details Contains de state variabes.
 * @note flag_ModeExecuted is used in the sate machine.
 */
typedef struct {
    Mode mode;                      /**< Current operational mode. */
    FlagStatus flag_ModeConfigured; /**< Flag to prevent re-configuration */
    Filter filter;                  /**< Active signal filter. */
    FlagStatus flag_readyToDisplay; /**< Whether the display refresh timer has fired **/
} System_T;

/**
 * @brief Global orchestrator instance.
 */
extern System_T SYSTEM;

/**
 * @brief System init.
 * @param mode The mode in which system inits.
 */
void system_init(Mode mode);

/**
 * @brief Sets a new operational mode.
 * @param mode the new mode.
 */
void system_setMode(Mode mode);
