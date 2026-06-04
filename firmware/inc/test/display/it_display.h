/**
 * Integration tests for the display canvas layer (src/display/display.c).
 */

#pragma once

/**
 * @brief Bring up the display subsystem.
 *
 * Call once before running any display-subsystem tests.
 */
void it_display_setup(void);

/**
 * @brief Run the display canvas-layer integration tests.
 */
void it_display_run(void);
