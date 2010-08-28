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

#ifdef S_SPLINT_S
# define COLORMGMTCAPS 1
# define CM_GAMMA_RAMP 1
#endif

/**\brief Win32 GDI state info */
typedef /*@partial@*/ struct {
	/**\brief HDC context */
	/*@null@*//*@partial@*/ HDC hDC;
	/**\brief Saved ramps */
	/*@null@*//*@partial@*/ WORD *saved_ramps;
} w32gdi_state_t;

#define GAMMA_RAMP_SIZE  256

static w32gdi_state_t state={NULL,NULL};

static int w32gdi_init(/*@unused@*/int screen_num,/*@unused@*/ int crtc_num)
{
	int cmcap;

	/* Open device context */
	if(state.hDC)
		(void)ReleaseDC(NULL, state.hDC);
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
	if(state.saved_ramps)
		free(state.saved_ramps);
	state.saved_ramps = malloc(3*GAMMA_RAMP_SIZE*sizeof(WORD));
	if (state.saved_ramps == NULL) {
		perror("malloc");
		(void)ReleaseDC(NULL, state.hDC);
		return RET_FUN_FAILED;
	}

	/* Save current gamma ramps so we can restore them at program exit */
	if( !GetDeviceGammaRamp(state.hDC, state.saved_ramps) ){
		LOG(LOGERR,_("Unable to save current gamma ramp."));
		(void)ReleaseDC(NULL, state.hDC);
		return RET_FUN_FAILED;
	}

	return RET_FUN_SUCCESS;
}

static int w32gdi_free(void)
{
	/* Free saved ramps */
	free(state.saved_ramps);

	/* Release device context */
	if( state.hDC )
		(void)ReleaseDC(NULL, state.hDC);
	return RET_FUN_SUCCESS;
}

static int w32gdi_restore(void)
{
	/* Restore gamma ramps */
	if( (!state.hDC)||(!state.saved_ramps) ){
		LOG(LOGERR,_("No device context or ramp."));
		return RET_FUN_FAILED;
	}
	if( !SetDeviceGammaRamp(state.hDC, state.saved_ramps) ){
		LOG(LOGERR,_("Unable to restore gamma ramps."));
		return RET_FUN_FAILED;
	}
	return RET_FUN_SUCCESS;
}

static int w32gdi_set_temperature(int temp, /*@unused@*/ gamma_s gamma)
{
	gamma_ramp_s ramp=gamma_ramp_fill(GAMMA_RAMP_SIZE,temp);

	/* Set new gamma ramps */
	if( (!state.hDC)||(!ramp.all) ){
		LOG(LOGERR,_("No device context or ramp."));
		return RET_FUN_FAILED;
	}
	if( !SetDeviceGammaRamp(state.hDC,ramp.all)) {
		LOG(LOGERR,_("Unable to set gamma ramps."));
		return RET_FUN_FAILED;
	}

	return RET_FUN_SUCCESS;
}

static int w32gdi_get_temperature(void){
	gamma_ramp_s ramp=gamma_get_ramps(GAMMA_RAMP_SIZE);
	float rb_ratio;
	
	if( (!state.hDC)||(!ramp.all) ){
		LOG(LOGERR,_("No device context or ramp."));
		return RET_FUN_FAILED;
	}

	if( !GetDeviceGammaRamp(state.hDC,ramp.all) ){
		LOG(LOGERR,_("Unable to get gamma ramps."));
		return RET_FUN_FAILED;
	}
	if( (!ramp.r)||(!ramp.b) ){
		LOG(LOGERR,_("No ramps found."));
		return RET_FUN_FAILED;
	}
	rb_ratio = (float)ramp.r[255]/(float)ramp.b[255];
	return gamma_find_temp(rb_ratio);
}

int w32gdi_load_funcs(gamma_method_s *method){
	method->func_init = &w32gdi_init;
	method->func_end = &w32gdi_free;
	method->func_set_temp = &w32gdi_set_temperature;
	method->func_get_temp = &w32gdi_get_temperature;
	method->func_restore = &w32gdi_restore;
	method->name = "WinGDI";
	return RET_FUN_SUCCESS;
}

