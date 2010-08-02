/**\file		vidmode.h
 * \author		Mao Yu,Jon Lund Steffensen
 * \date		Modified: Saturday, July 10, 2010
 * \brief		VidMode interface
 */

#ifndef _REDSHIFT_VIDMODE_H
#define _REDSHIFT_VIDMODE_H
#ifdef ENABLE_VIDMODE

#include <stdint.h>
#include "gamma.h"
#include <X11/Xlib.h>

/**\brief VidMode state storage */
typedef struct {
	/**\brief Display pointer */
	Display *display;
	/**\brief Screen number in use */
	int screen_num;
	/**\brief Size of the gamma ramps */
	int ramp_size;
	/**\brief Saved ramps */
	uint16_t *saved_ramps;
} vidmode_state_t;

/**\brief Initialize VidMode */
int vidmode_init(vidmode_state_t *state, int screen_num);

/**\brief Frees VidMode state */
void vidmode_free(vidmode_state_t *state);

/**\brief Restores saved gamma ramps */
void vidmode_restore(vidmode_state_t *state);

/**\brief Sets temperature using VidMode */
int vidmode_set_temperature(vidmode_state_t *state, int temp, gamma_s gamma);

/**\brief Retrieves temperature using VidMode */
int vidmode_get_temperature(vidmode_state_t *state);

#endif /*ENABLE_VIDMODE*/
#endif /* ! _REDSHIFT_VIDMODE_H */
