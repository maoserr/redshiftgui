/**\file		iupgui_gamma.h */

#ifndef __IUPGUI_GAMMA_H__
#define __IUPGUI_GAMMA_H__

/**\brief Returns current temperature value as known by GUI */
int guigamma_get_temp(void);

/**\brief Initializes timers to change gamma */
void guigamma_init_timers(void);

/**\brief Checks if temperatures need to be changed,
 * and starts transition timer if needed */
int guigamma_check(Ihandle *ih);

#endif//__IUPGUI_GAMMA_H__

