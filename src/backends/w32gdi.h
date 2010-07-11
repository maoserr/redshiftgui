/**\file		w32gdi.h
 * \author		Mao Yu,Jon Lund Steffensen
 * \date		Modified: Saturday, July 10, 2010
 * \brief		Win32 GDI interface
 */

#ifndef _REDSHIFT_W32GDI_H
#define _REDSHIFT_W32GDI_H

#include <wingdi.h>

/**\brief Win32 GDI state info */
typedef struct {
	/**\brief HDC context */
	HDC hDC;
	/**\brief Saved ramps */
	WORD *saved_ramps;
} w32gdi_state_t;

/**\brief Initialize windows GDI */
int w32gdi_init(w32gdi_state_t *state);

/**\brief Frees the GDI context */
void w32gdi_free(w32gdi_state_t *state);

/**\brief Restores saved gamma ramps */
void w32gdi_restore(w32gdi_state_t *state);

/**\brief Sets the temperature */
int w32gdi_set_temperature(w32gdi_state_t *state, int temp, gamma_s gamma);

/**\brief Retrieves current temperature */
int w32gdi_get_temperature(w32gdi_state_t *state);

#endif /* ! _REDSHIFT_W32GDI_H */
