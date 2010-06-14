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

/* Angular elevation of the sun at which the color temperature
   transition period starts and ends (in degress).
   Transition during twilight, and while the sun is lower than
   3.0 degrees above the horizon. */
#define TRANSITION_LOW     SOLAR_CIVIL_TWILIGHT_ELEV
#define TRANSITION_HIGH    3.0

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
/* Source: http://www.vendian.org/mncharity/dir3/blackbody/
   Rescaled to make exactly 6500K equal to full intensity in all channels. */
static const float blackbody_color[] = {
	1.0000f, 0.0425f, 0.0000f, /* 1000K */
	1.0000f, 0.0668f, 0.0000f, /* 1100K */
	1.0000f, 0.0911f, 0.0000f, /* 1200K */
	1.0000f, 0.1149f, 0.0000f, /* ... */
	1.0000f, 0.1380f, 0.0000f,
	1.0000f, 0.1604f, 0.0000f,
	1.0000f, 0.1819f, 0.0000f,
	1.0000f, 0.2024f, 0.0000f,
	1.0000f, 0.2220f, 0.0000f,
	1.0000f, 0.2406f, 0.0000f,
	1.0000f, 0.2630f, 0.0062f,
	1.0000f, 0.2868f, 0.0155f,
	1.0000f, 0.3102f, 0.0261f,
	1.0000f, 0.3334f, 0.0379f,
	1.0000f, 0.3562f, 0.0508f,
	1.0000f, 0.3787f, 0.0650f,
	1.0000f, 0.4008f, 0.0802f,
	1.0000f, 0.4227f, 0.0964f,
	1.0000f, 0.4442f, 0.1136f,
	1.0000f, 0.4652f, 0.1316f,
	1.0000f, 0.4859f, 0.1505f,
	1.0000f, 0.5062f, 0.1702f,
	1.0000f, 0.5262f, 0.1907f,
	1.0000f, 0.5458f, 0.2118f,
	1.0000f, 0.5650f, 0.2335f,
	1.0000f, 0.5839f, 0.2558f,
	1.0000f, 0.6023f, 0.2786f,
	1.0000f, 0.6204f, 0.3018f,
	1.0000f, 0.6382f, 0.3255f,
	1.0000f, 0.6557f, 0.3495f,
	1.0000f, 0.6727f, 0.3739f,
	1.0000f, 0.6894f, 0.3986f,
	1.0000f, 0.7058f, 0.4234f,
	1.0000f, 0.7218f, 0.4485f,
	1.0000f, 0.7375f, 0.4738f,
	1.0000f, 0.7529f, 0.4992f,
	1.0000f, 0.7679f, 0.5247f,
	1.0000f, 0.7826f, 0.5503f,
	1.0000f, 0.7970f, 0.5760f,
	1.0000f, 0.8111f, 0.6016f,
	1.0000f, 0.8250f, 0.6272f,
	1.0000f, 0.8384f, 0.6529f,
	1.0000f, 0.8517f, 0.6785f,
	1.0000f, 0.8647f, 0.7040f,
	1.0000f, 0.8773f, 0.7294f,
	1.0000f, 0.8897f, 0.7548f,
	1.0000f, 0.9019f, 0.7801f,
	1.0000f, 0.9137f, 0.8051f,
	1.0000f, 0.9254f, 0.8301f,
	1.0000f, 0.9367f, 0.8550f,
	1.0000f, 0.9478f, 0.8795f,
	1.0000f, 0.9587f, 0.9040f,
	1.0000f, 0.9694f, 0.9283f,
	1.0000f, 0.9798f, 0.9524f,
	1.0000f, 0.9900f, 0.9763f,
	1.0000f, 1.0000f, 1.0000f, /* 6500K */
	0.9771f, 0.9867f, 1.0000f,
	0.9554f, 0.9740f, 1.0000f,
	0.9349f, 0.9618f, 1.0000f,
	0.9154f, 0.9500f, 1.0000f,
	0.8968f, 0.9389f, 1.0000f,
	0.8792f, 0.9282f, 1.0000f,
	0.8624f, 0.9179f, 1.0000f,
	0.8465f, 0.9080f, 1.0000f,
	0.8313f, 0.8986f, 1.0000f,
	0.8167f, 0.8895f, 1.0000f,
	0.8029f, 0.8808f, 1.0000f,
	0.7896f, 0.8724f, 1.0000f,
	0.7769f, 0.8643f, 1.0000f,
	0.7648f, 0.8565f, 1.0000f,
	0.7532f, 0.8490f, 1.0000f,
	0.7420f, 0.8418f, 1.0000f,
	0.7314f, 0.8348f, 1.0000f,
	0.7212f, 0.8281f, 1.0000f,
	0.7113f, 0.8216f, 1.0000f,
	0.7018f, 0.8153f, 1.0000f,
	0.6927f, 0.8092f, 1.0000f,
	0.6839f, 0.8032f, 1.0000f,
	0.6755f, 0.7975f, 1.0000f,
	0.6674f, 0.7921f, 1.0000f,
	0.6595f, 0.7867f, 1.0000f,
	0.6520f, 0.7816f, 1.0000f,
	0.6447f, 0.7765f, 1.0000f,
	0.6376f, 0.7717f, 1.0000f,
	0.6308f, 0.7670f, 1.0000f,
	0.6242f, 0.7623f, 1.0000f,
	0.6179f, 0.7579f, 1.0000f,
	0.6117f, 0.7536f, 1.0000f,
	0.6058f, 0.7493f, 1.0000f,
	0.6000f, 0.7453f, 1.0000f,
	0.5944f, 0.7414f, 1.0000f /* 10000K */
};

static void
interpolate_color(float a, const float *c1, const float *c2, float *c)
{
	c[0] = (1.0-a)*c1[0] + a*c2[0];
	c[1] = (1.0-a)*c1[1] + a*c2[1];
	c[2] = (1.0-a)*c1[2] + a*c2[2];
}

void
colorramp_fill(uint16_t *gamma_r, uint16_t *gamma_g, uint16_t *gamma_b,
	       int size, int temp, gamma_s gamma)
{
	int i;
	/* Calculate white point */
	float white_point[3];
	float alpha = (temp % 100) / 100.0;
	int temp_index = ((temp - 1000) / 100)*3;
	interpolate_color(alpha, &blackbody_color[temp_index],
			  &blackbody_color[temp_index+3], white_point);

	for (i = 0; i < size; i++) {
		gamma_r[i] = pow((float)i/size, 1.0/gamma.r) *
			UINT16_MAX * white_point[0];
		gamma_g[i] = pow((float)i/size, 1.0/gamma.g) *
			UINT16_MAX * white_point[1];
		gamma_b[i] = pow((float)i/size, 1.0/gamma.b) *
			UINT16_MAX * white_point[2];
	}
}

/* Initialize gamma adjustment method. If method is negative
   try all methods until one that works is found. */
gamma_method_t gamma_init_method(int screen_num, int crtc_num,
		gamma_method_t method)
{
	int r;
#ifdef ENABLE_RANDR
	if (method < 0 || method == GAMMA_METHOD_RANDR) {
		/* Initialize RANDR state */
		r = randr_init(&(state.randr), screen_num, crtc_num);
		if (r < 0) {
			LOG(LOGERR,_("Initialization of RANDR failed.\n"));
			if (method < 0) {
				LOG(LOGINFO,_("Trying other method...\n"));
			} else {
				return -1;
			}
		} else {
			method = GAMMA_METHOD_RANDR;
		}
	}
#endif

#ifdef ENABLE_VIDMODE
	if (method < 0 || method == GAMMA_METHOD_VIDMODE) {
		/* Initialize VidMode state */
		r = vidmode_init(&(state.vidmode), screen_num);
		if (r < 0) {
			LOG(LOGERR,_("Initialization of VidMode failed.\n"));
			if (method < 0) {
				LOG(LOGERR,_("Trying other method...\n"));
			} else {
				return -1;
			}
		} else {
			method = GAMMA_METHOD_VIDMODE;
		}
	}
#endif

#ifdef ENABLE_WINGDI
	if (method < 0 || method == GAMMA_METHOD_WINGDI) {
		/* Initialize WinGDI state */
		r = w32gdi_init(&(state.w32gdi));
		if (r < 0) {
			LOG(LOGERR,_("Initialization of WinGDI failed.\n"));
			if (method < 0) {
				LOG(LOGERR,_("Trying other method...\n"));
			} else {
				return -1;
			}
		} else {
			method = GAMMA_METHOD_WINGDI;
		}
	}
#endif

	/* Failure if no methods were successful at this point. */
	if (method < 0) {
		LOG(LOGERR,_("No more methods to try.\n"));
		return -1;
	}
	return method;
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

	return -1;
}


/* Calculate color temperature for the specified solar elevation. */
int calculate_temp(double elevation, int temp_day, int temp_night, int verbose)
{
	int temp = 0;
	if (elevation < TRANSITION_LOW) {
		temp = temp_night;
		if (verbose) printf(_("Period: Night\n"));
	} else if (elevation < TRANSITION_HIGH) {
		/* Transition period: interpolate */
		float a = (TRANSITION_LOW - elevation) /
			(TRANSITION_LOW - TRANSITION_HIGH);
		temp = (1.0-a)*temp_night + a*temp_day;
		if (verbose) {
			printf(_("Period: Transition (%.2f%% day)\n"), a*100);
		}
	} else {
		temp = temp_day;
		if (verbose) printf(_("Period: Daytime\n"));
	}

	return temp;
}

/* Change gamma and exit. */
int do_oneshot(gamma_method_t method,float lat,float lon,
		gamma_s gamma, int temp_day, int temp_night, int verbose)
{
	double now,elevation;
	int r,temp;

	r = systemtime_get_time(&now);
	if (r < 0) {
		LOG(LOGERR,_("Unable to read system time.\n"));
		gamma_state_free(method);
		exit(EXIT_FAILURE);
	}

	/* Current angular elevation of the sun */
	elevation = solar_elevation(now, lat, lon);

	if (verbose) {
		/* TRANSLATORS: Append degree symbol if possible. */
		LOG(LOGINFO,_("Solar elevation: %f\n"));
	}

	/* Use elevation of sun to set color temperature */
	temp = calculate_temp(elevation, temp_day, temp_night, verbose);

	if (verbose) LOG(LOGINFO,_("Color temperature: %uK\n"), temp);

	/* Adjust temperature */
	r = gamma_state_set_temperature(method, temp, gamma);
	if (r < 0) {
		LOG(LOGERR,_("Temperature adjustment failed.\n"));
		gamma_state_free(method);
		return -1;;
	}
	gamma_state_free(method);
	return 0;
}

