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

/** Enum of gamma adjustment methods */
typedef enum {
	GAMMA_METHOD_NONE,
	GAMMA_METHOD_RANDR,
	GAMMA_METHOD_VIDMODE,
	GAMMA_METHOD_WINGDI,
	GAMMA_METHOD_MAX
} gamma_method_t;

/** Fills gamma ramp with linearly interpolated values */
void gamma_ramp_fill(uint16_t *gamma_r, uint16_t *gamma_g,
		uint16_t *gamma_b, int size, int temp, gamma_s gamma);

/** Find the temperature based on red:blue ratio */
int gamma_find_temp(float ratio);

/** Initialize gamma changing method. */
gamma_method_t gamma_init_method(int screen_num, int crtc_num,
		gamma_method_t method);

/** Restores saved gamma state. */
void gamma_state_restore(gamma_method_t method);

/** Free the state associated with the appropriate adjustment method. */
void gamma_state_free(gamma_method_t method);

/** Calculate temperature based on elevation. */
int gamma_calc_temp(double elevation, int temp_day, int temp_night);

/** Sets the temperature */
int gamma_state_set_temperature(gamma_method_t method,
			    int temp, gamma_s gamma);

/** Retrieves current temperature */
int gamma_state_get_temperature(gamma_method_t method);

#endif//__GAMMA_H__
