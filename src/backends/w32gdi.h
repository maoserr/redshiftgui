/**\file		w32gdi.h
 * \author		Mao Yu,Jon Lund Steffensen
 * \date		Modified: Saturday, July 10, 2010
 * \brief		Win32 GDI interface
 */

#ifndef _REDSHIFT_W32GDI_H
#define _REDSHIFT_W32GDI_H
#ifdef ENABLE_WINGDI

#include <wingdi.h>

/**\brief Initialize windows GDI */
int w32gdi_init(int screen_num,int crtc_num);

/**\brief Frees the GDI context */
int w32gdi_free(void);

/**\brief Restores saved gamma ramps */
void w32gdi_restore(void);

/**\brief Sets the temperature */
int w32gdi_set_temperature(int temp, gamma_s gamma);

/**\brief Retrieves current temperature */
int w32gdi_get_temperature(void);

/**\brief Load WinGDI functions into methods structure */
int w32gdi_load_funcs(gamma_method_s *method);

#endif /*ENABLE_WINGDI*/
#endif /* ! _REDSHIFT_W32GDI_H */
