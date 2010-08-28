/**\file		w32gdi.h
 * \author		Mao Yu,Jon Lund Steffensen
 * \date		Modified: Saturday, July 10, 2010
 * \brief		Win32 GDI interface
 */

#ifndef _REDSHIFT_W32GDI_H
#define _REDSHIFT_W32GDI_H
#ifdef ENABLE_WINGDI

#include <wingdi.h>

/**\brief Load WinGDI functions into methods structure */
int w32gdi_load_funcs(gamma_method_s *method);

#endif /*ENABLE_WINGDI*/
#endif /* ! _REDSHIFT_W32GDI_H */
