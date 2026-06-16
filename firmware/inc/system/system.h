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
    MODE_COUNT,
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

/**
 * @brief for each mode to register its implementation
 */
void system_registerMode(Mode mode, SystemMode_T const* hooks);
void realTimeMode_registerHooks(void);
void noiseSamplingMode_registerHooks(void);
void eqMode_registerHooks(void);

/**
 * @brief The orchestrator struct.
 * @details Contains de state variables.
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

/**
 * @brief executes the current mode
 */
void system_tick(void);

/**
 * @brief consumes a keypress
 * @details if the key is a mode change, handles the mode changes. Otherwise, it passes the key to
 * the current mode to be consumed.
 * @param c
 */
void system_handleKey(char c);
