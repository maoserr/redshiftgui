/**\file		gamma.h
 * \author		Mao Yu
 * \brief		Gamma/temperature adjustment functions.
 */

#ifndef __GAMMA_H__
#define __GAMMA_H__

#include <stdint.h>

/** gamma structure */
typedef struct{
	float r;
	float g;
	float b;
} gamma_s;

#ifdef ENABLE_RANDR
# include "backends/randr.h"
#endif
#ifdef ENABLE_VIDMODE
# include "backends/vidmode.h"
#endif
#ifdef ENABLE_WINGDI
# include "backends/w32gdi.h"
#endif

/** Enum of gamma adjustment methods */
typedef enum {
	GAMMA_METHOD_RANDR,
	GAMMA_METHOD_VIDMODE,
	GAMMA_METHOD_WINGDI,
	GAMMA_METHOD_MAX
} gamma_method_t;

void colorramp_fill(uint16_t *gamma_r, uint16_t *gamma_g, uint16_t *gamma_b,
		    int size, int temp, gamma_s gamma);

gamma_method_t gamma_init_method(int screen_num, int crtc_num, gamma_method_t method);

void gamma_state_restore(gamma_method_t method);

/* Free the state associated with the appropriate adjustment method. */
void gamma_state_free(gamma_method_t method);

int gamma_state_set_temperature(gamma_method_t method,
			    int temp, float gamma[3]);

int calculate_temp(double elevation, int temp_day, int temp_night, int verbose);

int do_oneshot(gamma_method_t method,float lat,float lon,
		gamma_s gamma, int temp_day, int temp_night, int verbose);

#endif//__GAMMA_H__
