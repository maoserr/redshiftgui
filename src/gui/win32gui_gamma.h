/**\file		win32gui_gamma.h 
 * \author		Mao Yu
 * \date		Modified: Saturday, June 7, 2014
 * \brief		GUI gamma adjustment functions
 */

#ifndef __WIN32GUI_GAMMA_H__
#define __WIN32GUI_GAMMA_H__

/**\brief Returns current temperature value as known by GUI */
int guigamma_get_temp(void);

/**\brief Sets the current temperature in GUI mode */
int guigamma_set_temp(int temp);

/**\brief Disables gamma timers */
void guigamma_disable(void);

/**\brief Enables gamma timers */
void guigamma_enable(void);

/**\brief Initializes timers to change gamma */
void guigamma_init_timers(void);

/**\brief Destroys timers */
void guigamma_end_timers(void);

/**\brief Checks if temperatures need to be changed,
 * and starts transition timer if needed */
void guigamma_check(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime);

#endif//__WIN32GUI_GAMMA_H__

