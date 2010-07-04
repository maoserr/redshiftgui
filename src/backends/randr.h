/**\file		randr.h
 * \author		Mao Yu
 * \date		Friday, June 11, 2010
 * \brief		Main code
 * \details
 * This code is forked from the redshift project
 * (https://bugs.launchpad.net/redshift) by:
 * Jon Lund Steffensen.
 *
 * The license for this project as a whole is same (GPL v3),
 * although some components of this code (such as argument parsing)
 * were originally under different license.
 */

#ifndef _REDSHIFT_RANDR_H
#define _REDSHIFT_RANDR_H

#include "gamma.h"
#include <stdint.h>

#include <xcb/xcb.h>
#include <xcb/randr.h>

typedef struct {
	xcb_randr_crtc_t crtc;
	unsigned int ramp_size;
	uint16_t *saved_ramps;
} randr_crtc_state_t;

typedef struct {
	xcb_connection_t *conn;
	xcb_screen_t *screen;
	int crtc_num;
	unsigned int crtc_count;
	randr_crtc_state_t *crtcs;
} randr_state_t;


int randr_init(randr_state_t *state, int screen_num, int crtc_num);
void randr_free(randr_state_t *state);
void randr_restore(randr_state_t *state);
int randr_set_temperature(randr_state_t *state, int temp, gamma_s gamma);
int randr_get_temperature(randr_state_t *state);


#endif /* ! _REDSHIFT_RANDR_H */
