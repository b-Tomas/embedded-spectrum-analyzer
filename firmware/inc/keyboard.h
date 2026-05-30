#if !defined(KEYBOARD_H)
#define KEYBOARD_H

/* @brief PINSEL, GPIO and NVIC setting
 *
 * @note
 *
 * 'A', 'B', 'C' select the post-filter
 *
 * 'D' toggle between real time and noise sampling mode
 *
 * '#' Change to Equelizer mode
 */
void KEYBOARD_setup(void);

/* @brief Keyboard config when mode is Equalizator mode.
 */
void KEYBOARD_setEqualizerMode(void);

/* @brief Keyboard config when mode is in Configuration mode (filter selecting).
 */
void KEYBOARD_setConfigurationMode(void);

#endif // KEYBOARD_H