/**\file		randr.h
 * \author		Mao Yu,Jon Lund Steffensen
 * \date		Modified: Saturday, July 10, 2010
 * \brief		Randr interface
 */

#ifndef _REDSHIFT_RANDR_H
#define _REDSHIFT_RANDR_H
#ifdef ENABLE_RANDR

#include "gamma.h"
#include <stdint.h>

#include <xcb/xcb.h>
#include <xcb/randr.h>

/**\brief Randr storage of CRTC state info */
typedef struct {
	/**\brief CRTC number */
	xcb_randr_crtc_t crtc;
	/**\brief Length of gamma ramps */
	unsigned int ramp_size;
	/**\brief Pointer to saved gamma ramps */
	uint16_t *saved_ramps;
} randr_crtc_state_t;

/**\brief Randr storage of state info */
typedef struct {
	/**\brief XCB connection pointer */
	xcb_connection_t *conn;
	/**\brief XCB screen pointer */
	xcb_screen_t *screen;
	/**\brief CRTC number */
	int crtc_num;
	/**\brief Number of CRTC */
	unsigned int crtc_count;
	/**\brief State of CRTCs*/
	randr_crtc_state_t *crtcs;
} randr_state_t;

/**\brief Initialize Randr */
int randr_init(randr_state_t *state, int screen_num, int crtc_num);

/**\brief Frees Randr */
void randr_free(randr_state_t *state);

/**\brief Restores saved gamma ramps */
void randr_restore(randr_state_t *state);

/**\brief Sets the temperature using Randr */
int randr_set_temperature(randr_state_t *state, int temp, gamma_s gamma);

/**\brief Retrieves the temperature
 * \bug Sometimes Randr returns 6500K even when it's not
 */
int randr_get_temperature(randr_state_t *state);


#endif /*ENABLE_RANDR*/
#endif /* ! _REDSHIFT_RANDR_H */
