/**\file		gamma.c
 * \author		Mao Yu
 * \brief		Gamma/temperature adjustment functions.
 */

#if !(defined(ENABLE_RANDR) ||			\
      defined(ENABLE_VIDMODE) ||		\
      defined(ENABLE_WINGDI))
# error "At least one of RANDR, VidMode or WinGDI must be enabled."
#endif

#include "common.h"
#include "gamma.h"
#include "solar.h"
#include "systemtime.h"

#ifdef ENABLE_RANDR
# include "backends/randr.h"
#endif
#ifdef ENABLE_VIDMODE
# include "backends/vidmode.h"
#endif
#ifdef ENABLE_WINGDI
# include "backends/w32gdi.h"
#endif

#include "gamma_vals.h"

/* Angular elevation of the sun at which the color temperature
   transition period starts and ends (in degress).
   Transition during twilight, and while the sun is lower than
   3.0 degrees above the horizon. */
#define TRANSITION_LOW     SOLAR_CIVIL_TWILIGHT_ELEV
#define TRANSITION_HIGH    3.0f

/* Union of state data for gamma adjustment methods */
typedef union {
#ifdef ENABLE_RANDR
	randr_state_t randr;
#endif
#ifdef ENABLE_VIDMODE
	vidmode_state_t vidmode;
#endif
#ifdef ENABLE_WINGDI
	w32gdi_state_t w32gdi;
#endif
} gamma_state_t;

static gamma_state_t state;

static void gamma_interp_color(float a,
		const float *c1, const float *c2, float *c)
{
	c[0] = (1.0f-a)*c1[0] + a*c2[0];
	c[1] = (1.0f-a)*c1[1] + a*c2[1];
	c[2] = (1.0f-a)*c1[2] + a*c2[2];
}

char *gamma_get_method_name(gamma_method_t method){
	switch( method ){
		case GAMMA_METHOD_RANDR:
			return _("RANDR");
		case GAMMA_METHOD_VIDMODE:
			return _("VidMode");
		case GAMMA_METHOD_WINGDI:
			return _("WinGDI");
		default:
			return _("Unknown");
	}
}

void gamma_ramp_fill(uint16_t *gamma_r, uint16_t *gamma_g,
		uint16_t *gamma_b, int size, int temp, gamma_s gamma)
{
	int i;
	/* Calculate white point */
	float white_point[3];
	float alpha = (temp % 100) / 100.0f;
	int temp_index = ((temp - 1000) / 100)*3;
	gamma_interp_color(alpha, &blackbody_color[temp_index],
			  &blackbody_color[temp_index+3], white_point);

	for (i = 0; i < size; i++) {
		gamma_r[i] = (uint16_t)(pow((float)i/size, 1.0f/gamma.r) *
			UINT16_MAX * white_point[0]);
		gamma_g[i] = (uint16_t)(pow((float)i/size, 1.0f/gamma.g) *
			UINT16_MAX * white_point[1]);
		gamma_b[i] = (uint16_t)(pow((float)i/size, 1.0f/gamma.b) *
			UINT16_MAX * white_point[2]);
	}
}

int gamma_find_temp(float ratio){
	int i;
	int gam_val_size=SIZEOF(blackbody_color);
	float curr_ratio;
	LOG(LOGVERBOSE,_("R/B Ratio: %f"),ratio);
	for(i=0; i<gam_val_size; ++i){
		curr_ratio = (float)blackbody_color[i*3]/(float)blackbody_color[i*3+2];
		if( curr_ratio <= ratio ){
			LOG(LOGVERBOSE,_("Current col:%d"),i*100+1000);
			return (i*100+1000);
		}
	}
	return RET_FUN_FAILED;
}


/* Initialize gamma adjustment method. If method is negative
   try all methods until one that works is found. */
gamma_method_t gamma_init_method(int screen_num, int crtc_num,
		gamma_method_t method)
{
	gamma_method_t vmethod=method;	// The valid method found
#ifdef ENABLE_RANDR
	if (vmethod == GAMMA_METHOD_NONE || vmethod == GAMMA_METHOD_RANDR) {
		/* Initialize RANDR state */
		if( !randr_init(&(state.randr), screen_num, crtc_num) ){
			LOG(LOGERR,_("Initialization of RANDR failed."));
			if (vmethod == GAMMA_METHOD_NONE)
				LOG(LOGINFO,_("Trying other method..."));
			else
				 return GAMMA_METHOD_NONE;
		} else
			vmethod = GAMMA_METHOD_RANDR;
	}
#endif

#ifdef ENABLE_VIDMODE
	if (vmethod == GAMMA_METHOD_NONE || vmethod == GAMMA_METHOD_VIDMODE) {
		/* Initialize VidMode state */
		if( !vidmode_init(&(state.vidmode), screen_num) ){
			LOG(LOGERR,_("Initialization of VidMode failed."));
			if (vmethod == GAMMA_METHOD_NONE)
				LOG(LOGERR,_("Trying other method..."));
			else
				return GAMMA_METHOD_NONE;
		} else
			vmethod = GAMMA_METHOD_VIDMODE;
	}
#endif

#ifdef ENABLE_WINGDI
	if (vmethod == GAMMA_METHOD_NONE || vmethod == GAMMA_METHOD_WINGDI) {
		/* Initialize WinGDI state */
		if( !w32gdi_init(&(state.w32gdi)) ){
			LOG(LOGERR,_("Initialization of WinGDI failed."));
			if (vmethod == GAMMA_METHOD_NONE)
				LOG(LOGERR,_("Trying other method..."));
			else
				return GAMMA_METHOD_NONE;
		} else
			vmethod = GAMMA_METHOD_WINGDI;
	}
#endif

	/* Failure if no methods were successful at this point. */
	if (vmethod == GAMMA_METHOD_NONE) {
		LOG(LOGERR,_("No more methods to try."));
		return GAMMA_METHOD_NONE;
	}
	return vmethod;
}


/* Restore saved gamma ramps with the appropriate adjustment method. */
void gamma_state_restore(gamma_method_t method)
{
	switch (method) {
#ifdef ENABLE_RANDR
	case GAMMA_METHOD_RANDR:
		randr_restore(&state.randr);
		break;
#endif
#ifdef ENABLE_VIDMODE
	case GAMMA_METHOD_VIDMODE:
		vidmode_restore(&state.vidmode);
		break;
#endif
#ifdef ENABLE_WINGDI
	case GAMMA_METHOD_WINGDI:
		w32gdi_restore(&state.w32gdi);
		break;
#endif
	default:
		break;
	}
}

/* Free the state associated with the appropriate adjustment method. */
void gamma_state_free(gamma_method_t method)
{
	switch (method) {
#ifdef ENABLE_RANDR
	case GAMMA_METHOD_RANDR:
		randr_free(&state.randr);
		break;
#endif
#ifdef ENABLE_VIDMODE
	case GAMMA_METHOD_VIDMODE:
		vidmode_free(&state.vidmode);
		break;
#endif
#ifdef ENABLE_WINGDI
	case GAMMA_METHOD_WINGDI:
		w32gdi_free(&state.w32gdi);
		break;
#endif
	default:
		break;
	}
}

/* Calculate color temperature for the specified solar elevation. */
int gamma_calc_temp(double elevation, int temp_day, int temp_night)
{
	int temp = 0;
	if (elevation < TRANSITION_LOW) {
		temp = temp_night;
		LOG(LOGINFO,_("Period: Night"));
	} else if (elevation < TRANSITION_HIGH) {
		/* Transition period: interpolate */
		double a = (TRANSITION_LOW - elevation) /
			(TRANSITION_LOW - TRANSITION_HIGH);
		temp = (int)((1.0-a)*temp_night + a*temp_day);
		LOG(LOGINFO,_("Period: Transition (%.2f%% day)"), a*100);
	} else {
		temp = temp_day;
		LOG(LOGINFO,_("Period: Daytime"));
	}

	return temp;
}

/* Calculates the current target temperature */
int gamma_calc_curr_target_temp(float lat, float lon,
		int temp_day, int temp_night)
{
	double now, elevation;
	int temp;
	if ( !systemtime_get_time(&now) ){
		LOG(LOGERR,_("Unable to read system time."));
		return RET_FUN_FAILED;
	}
	/* Current angular elevation of the sun */
	elevation = solar_elevation(now, lat, lon);
	/* TRANSLATORS: Append degree symbol if possible. */
	LOG(LOGINFO,_("Solar elevation: %f"),elevation);
	/* Use elevation of sun to set color temperature */
	temp = gamma_calc_temp(elevation, temp_day,temp_night);
	LOG(LOGINFO,_("Calculated temp: %d"),temp);
	return temp;
}

/* Set temperature with the appropriate adjustment method. */
int gamma_state_set_temperature(gamma_method_t method,
			    int temp, gamma_s gamma)
{
	switch (method) {
#ifdef ENABLE_RANDR
	case GAMMA_METHOD_RANDR:
		return randr_set_temperature(&state.randr, temp, gamma);
#endif
#ifdef ENABLE_VIDMODE
	case GAMMA_METHOD_VIDMODE:
		return vidmode_set_temperature(&state.vidmode, temp, gamma);
#endif
#ifdef ENABLE_WINGDI
	case GAMMA_METHOD_WINGDI:
		return w32gdi_set_temperature(&state.w32gdi, temp, gamma);
#endif
	default:
		break;
	}

	return RET_FUN_FAILED;
}

/* Retrieves temperature with the appropriate adjustment method. */
int gamma_state_get_temperature(gamma_method_t method){
	switch (method) {
#ifdef ENABLE_RANDR
	case GAMMA_METHOD_RANDR:
		return randr_get_temperature(&state.randr);
#endif
#ifdef ENABLE_VIDMODE
	case GAMMA_METHOD_VIDMODE:
		return vidmode_get_temperature(&state.vidmode);
#endif
#ifdef ENABLE_WINGDI
	case GAMMA_METHOD_WINGDI:
		return w32gdi_get_temperature(&state.w32gdi);
#endif
	default:
		break;
	}
	return RET_FUN_FAILED;
}

