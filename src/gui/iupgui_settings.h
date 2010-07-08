/**\file		iupgui_settings.h */

#ifndef __IUPGUI_SETTINGS_H__
#define __IUPGUI_SETTINGS_H__

/**\brief Creates or shows the settings dialog, can be called multiple times */
int guisettings_show(Ihandle *ih);

/**\brief Closes the settings dialog, function is ignored dialog not open */
int guisettings_close(Ihandle *ih);

#endif//__IUPGUI_SETTINGS_H__
