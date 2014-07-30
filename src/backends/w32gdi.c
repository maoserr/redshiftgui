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
	/**\brief Saved ramps */
	/*@null@*//*@partial@*/ WORD *saved_ramps;
} w32gdi_state_t;

#define GAMMA_RAMP_SIZE  256

static w32gdi_state_t state={NULL};

static int w32gdi_init(/*@unused@*/int screen_num,/*@unused@*/ int crtc_num)
{
	int cmcap;

	int n,i;
	HDC hdc;
	DISPLAY_DEVICE dd;
	n = GetSystemMetrics(SM_CMONITORS);
	LOG(LOGVERBOSE,_("Found %d monitors."),n);
	dd.cb = sizeof(dd);

   for (i=0;i<n;i++) {
      dd.DeviceName  [0] = '\0';
      dd.DeviceString[0] = '\0';
      dd.DeviceID    [0] = '\0';
      dd.DeviceKey   [0] = '\0';

      dd.StateFlags   = 0;

      EnumDisplayDevices(NULL, i, &dd, 0);

      LOG(LOGVERBOSE,_("Screen %d  Flags: %d"), i);
	  LOG(LOGVERBOSE,_("Flags: %d"), dd.StateFlags);
      LOG(LOGVERBOSE,_("Name: %s"),  dd.DeviceName);
      LOG(LOGVERBOSE,_("String: %s"),dd.DeviceString);
      LOG(LOGVERBOSE,_("ID: %s"),    dd.DeviceID);
      LOG(LOGVERBOSE,_("Key: %s"),   dd.DeviceKey);

      hdc = CreateDC(NULL, dd.DeviceName, NULL, NULL);
      LOG(LOGVERBOSE,_("  dims: %dx%d"), GetDeviceCaps(hdc, HORZRES), GetDeviceCaps(hdc, VERTRES));
      LOG(LOGVERBOSE,_("  colors: %d bits"), GetDeviceCaps(hdc, BITSPIXEL));
	  cmcap = GetDeviceCaps(hdc, COLORMGMTCAPS);
	  (void)DeleteDC(hdc);
   }


	/* Open device context */
	hdc = CreateDC(TEXT("DISPLAY"),NULL,NULL,NULL);
	if (hdc == NULL) {
		LOG(LOGERR,_("Unable to open device context."));
		return RET_FUN_FAILED;
	}

	/* Check support for gamma ramps */
	cmcap = GetDeviceCaps(hdc, COLORMGMTCAPS);
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
		(void)DeleteDC(hdc);
		return RET_FUN_FAILED;
	}

	/* Save current gamma ramps so we can restore them at program exit */
	if( !GetDeviceGammaRamp(hdc, state.saved_ramps) ){
		LOG(LOGERR,_("Unable to save current gamma ramp."));
		(void)DeleteDC(hdc);
		return RET_FUN_FAILED;
	}
	(void)DeleteDC(hdc);
	return RET_FUN_SUCCESS;
}

static int w32gdi_free(void)
{
	/* Free saved ramps */
	free(state.saved_ramps);

	return RET_FUN_SUCCESS;
}

static int w32gdi_restore(void)
{
	HDC hdc;
	hdc = CreateDC(TEXT("DISPLAY"),NULL,NULL,NULL);
	/* Restore gamma ramps */
	if( (hdc)||(!state.saved_ramps) ){
		LOG(LOGERR,_("No device context or ramp."));
		(void)DeleteDC(hdc);
		return RET_FUN_FAILED;
	}
	if( !SetDeviceGammaRamp(hdc, state.saved_ramps) ){
		LOG(LOGERR,_("Unable to restore gamma ramps."));
		(void)DeleteDC(hdc);
		return RET_FUN_FAILED;
	}
	return RET_FUN_SUCCESS;
}

static int w32gdi_set_temperature(int temp, /*@unused@*/ gamma_s gamma)
{
	HDC hdc;
	gamma_ramp_s ramp=gamma_ramp_fill(GAMMA_RAMP_SIZE,temp);

	/* Set new gamma ramps */
	hdc = CreateDC(TEXT("DISPLAY"),NULL,NULL,NULL);
	if( (!hdc)||(!ramp.all) ){
		LOG(LOGERR,_("No device context or ramp."));
	    (void)DeleteDC(hdc);
		return RET_FUN_FAILED;
	}

	if( !SetDeviceGammaRamp(hdc,ramp.all)) {
		LOG(LOGERR,_("Unable to set gamma ramps."));
	    (void)DeleteDC(hdc);
    }

	(void)DeleteDC(hdc);
	return RET_FUN_SUCCESS;
}

static int w32gdi_get_temperature(void){
	gamma_ramp_s ramp=gamma_get_ramps(GAMMA_RAMP_SIZE);
	float rb_ratio;
	HDC hdc;
	
	hdc = CreateDC(TEXT("DISPLAY"),NULL,NULL,NULL);
	if( (!hdc)||(!ramp.all) ){
		LOG(LOGERR,_("No device context or ramp."));
		(void)DeleteDC(hdc);
		return RET_FUN_FAILED;
	}

	if( !GetDeviceGammaRamp(hdc,ramp.all) ){
		LOG(LOGERR,_("Unable to get gamma ramps."));
		(void)DeleteDC(hdc);
		return RET_FUN_FAILED;
	}
	if( (!ramp.r)||(!ramp.b) ){
		LOG(LOGERR,_("No ramps found."));
		(void)DeleteDC(hdc);
		return RET_FUN_FAILED;
	}
	(void)DeleteDC(hdc);
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

