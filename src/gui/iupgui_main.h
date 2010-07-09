/**\file		iupgui_main.h */

#ifndef __IUPGUI_MAIN_H__
#define __IUPGUI_MAIN_H__

/** Updates info display */
void guimain_update_info(void);

/** Initializes main dialog */
void guimain_dialog_init( int show );

/** Sets main application exit status */
int guimain_set_exit(int exit);

/** Gets main application exit status */
int guimain_exit_normal(void);


#endif//__IUPGUI_MAIN_H__
