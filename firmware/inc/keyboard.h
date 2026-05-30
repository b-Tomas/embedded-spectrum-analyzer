#if !defined(KEYBOARD_H)
#define KEYBOARD_H

/* @brief Keyboard config when mode is Equalizator mode.
 */
void KEYBOARD_Mode3Configuration(void);

/* @brief Keyboard config when mode is in Configuration mode (filter selecting).
 */
void KEYBOARD_Mode4Configuration(void);

/* @brief for ensure that config has been run only once
 */
Bool KEYBOARD_Mode1Configurated(void);

/* @brief for ensure that config has been run only once
 */
Bool KEYBOARD_Mode4Configurated(void);

#endif // KEYBOARD_H