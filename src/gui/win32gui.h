/**\file		win32gui.h
 * \author		Mao Yu
 * \date		Modified: Saturday, June 7, 2014
 * \brief		Main Win32 entry point
 */

#ifndef __WIN32GUI_H__
#define __WIN32GUI_H__

/**\brief Main Win32 GUI loop */
int win32_gui(int argc, char *argv[]);

/**\brief Update GUI status display */
void guimain_update_info(void);

#endif//__WIN32GUI_H__