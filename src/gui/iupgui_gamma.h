/**\file		iupgui_gamma.h 
 * \author		Mao Yu
 * \date		Modified: Saturday, July 10, 2010
 * \brief		GUI gamma adjustment functions
 */

#ifndef __IUPGUI_GAMMA_H__
#define __IUPGUI_GAMMA_H__

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

/**\brief Checks if temperatures need to be changed,
 * and starts transition timer if needed */
int guigamma_check(Ihandle *ih);

#endif//__IUPGUI_GAMMA_H__

