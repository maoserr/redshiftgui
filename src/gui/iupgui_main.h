/**\file		iupgui_main.h
 * \author		Mao Yu
 * \date		Modified: Saturday, July 10, 2010
 * \brief		Main dialog
 */

#ifndef __IUPGUI_MAIN_H__
#define __IUPGUI_MAIN_H__

/**\brief Updates info display */
void guimain_update_info(void);

/**\brief Initializes main dialog
 * \param min Start minimized
 */
void guimain_dialog_init( int min );

/**\brief Sets main application exit status */
int guimain_set_exit(int exit);

/**\brief Gets main application exit status */
int guimain_exit_normal(void);


#endif//__IUPGUI_MAIN_H__
