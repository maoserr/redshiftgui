/* w32gdi.c -- Windows GDI gamma adjustment source
   This file is part of Redshift.

   Redshift is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Redshift is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Redshift.  If not, see <http://www.gnu.org/licenses/>.

   Copyright (c) 2010  Jon Lund Steffensen <jonlst@gmail.com>
*/

#include "common.h"
#include "gamma.h"
#include "w32gdi.h"

/**\brief Win32 GDI state info */
typedef struct {
	/**\brief HDC context */
	HDC hDC;
	/**\brief Saved ramps */
	WORD *saved_ramps;
} w32gdi_state_t;

#define GAMMA_RAMP_SIZE  256

int w32gdi_init(int screen_num,int crtc_num)
{
	int cmcap;

	/* Open device context */
	state.hDC = GetDC(NULL);
	if (state.hDC == NULL) {
		LOG(LOGERR,_("Unable to open device context."));
		return RET_FUN_FAILED;
	}

	/* Check support for gamma ramps */
	cmcap = GetDeviceCaps(state.hDC, COLORMGMTCAPS);
	if (cmcap != CM_GAMMA_RAMP) {
		LOG(LOGERR,_("Display device does not support gamma ramps."));
		return RET_FUN_FAILED;
	}

	/* Allocate space for saved gamma ramps */
	state.saved_ramps = malloc(3*GAMMA_RAMP_SIZE*sizeof(WORD));
	if (state.saved_ramps == NULL) {
		perror("malloc");
		ReleaseDC(NULL, state.hDC);
		return RET_FUN_FAILED;
	}

	/* Save current gamma ramps so we can restore them at program exit */
	if( !GetDeviceGammaRamp(state.hDC, state.saved_ramps) ){
		LOG(LOGERR,_("Unable to save current gamma ramp."));
		ReleaseDC(NULL, state.hDC);
		return RET_FUN_FAILED;
	}

	return RET_FUN_SUCCESS;
}

int w32gdi_free(void)
{
	/* Free saved ramps */
	free(state.saved_ramps);

	/* Release device context */
	ReleaseDC(NULL, state.hDC);
	return RET_FUN_SUCCESS;
}

void w32gdi_restore(void)
{
	/* Restore gamma ramps */
	if( !SetDeviceGammaRamp(state.hDC, state.saved_ramps) )
		LOG(LOGERR,_("Unable to restore gamma ramps."));
}

int w32gdi_set_temperature(int temp, gamma_s gamma)
{
	WORD *gamma_r, *gamma_g, *gamma_b;

	/* Create new gamma ramps */
	WORD *gamma_ramps = malloc(3*GAMMA_RAMP_SIZE*sizeof(WORD));
	if (gamma_ramps == NULL) {
		perror("malloc");
		return RET_FUN_FAILED;
	}

	gamma_r = &gamma_ramps[0*GAMMA_RAMP_SIZE];
	gamma_g = &gamma_ramps[1*GAMMA_RAMP_SIZE];
	gamma_b = &gamma_ramps[2*GAMMA_RAMP_SIZE];

	gamma_ramp_fill(gamma_r, gamma_g, gamma_b, GAMMA_RAMP_SIZE,
		       temp, gamma);

	/* Set new gamma ramps */
	if( !SetDeviceGammaRamp(state.hDC, gamma_ramps)) {
		LOG(LOGERR,_("Unable to set gamma ramps."));
		free(gamma_ramps);
		return RET_FUN_FAILED;
	}

	free(gamma_ramps);

	return RET_FUN_SUCCESS;
}

int w32gdi_get_temperature(void){
	WORD gamma_ramp[3][256];
	float rb_ratio;
	
	if( !GetDeviceGammaRamp(state.hDC,gamma_ramp) ){
		LOG(LOGERR,_("Unable to get gamma ramps."));
		return RET_FUN_FAILED;
	}
	rb_ratio = (float)gamma_ramp[0][255]/(float)gamma_ramp[2][255];
	return gamma_find_temp(rb_ratio);
}

int w32gdi_load_funcs(gamma_method_s *method){
	method->func_init = &w32gdi_init;
	method->func_end = &w32gdi_free;
	method->func_set_temp = &w32gdi_set_temperature;
	method->func_get_temp = &w32gdi_get_temperature;
	method->name = "WinGDI";
	return RET_FUN_SUCCESS;
}

