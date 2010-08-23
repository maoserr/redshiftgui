/**\file		iupgui.h
 * \author		Mao Yu
 * \date		Modified: Saturday, July 10, 2010
 * \brief		Main IUP entry point
 */

#ifndef __IUPGUI_H__
#define __IUPGUI_H__

#include <iup.h>

/**\brief All IUP objects are stored by IUP internally */
typedef /*@dependent@*/ Ihandle *Hcntrl;
/**\brief Handles which can be NULL */
typedef /*@null@*/ Hcntrl Hnullc;

/**\brief About dialog */
int gui_about(Ihandle *ih);

/**\brief Popups */
int gui_popup(char *title,char *msg,char *type);

/**\brief Main IUP GUI loop */
int iup_gui(int argc, char *argv[]);

#endif//__IUPGUI_H__
