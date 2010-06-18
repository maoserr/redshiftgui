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

gamma_state_t state;

/* Source f.lux */
#define USE_FLUX_VALS
#ifdef USE_FLUX_VALS
static const float blackbody_color[] = {
	1.0000f, 0.5470f, 0.2132f, /* 1000 */
	1.0000f, 0.5582f, 0.2258f, /* 1100 */
	1.0000f, 0.5693f, 0.2385f, /* 1200 */
	1.0000f, 0.5802f, 0.2515f, /* 1300 */
	1.0000f, 0.5910f, 0.2646f, /* 1400 */
	1.0000f, 0.6017f, 0.2779f, /* 1500 */
	1.0000f, 0.6122f, 0.2913f, /* 1600 */
	1.0000f, 0.6227f, 0.3049f, /* 1700 */
	1.0000f, 0.6330f, 0.3186f, /* 1800 */
	1.0000f, 0.6433f, 0.3325f, /* 1900 */
	1.0000f, 0.6534f, 0.3464f, /* 2000 */
	1.0000f, 0.6634f, 0.3605f, /* 2100 */
	1.0000f, 0.6732f, 0.3747f, /* 2200 */
	1.0000f, 0.6829f, 0.3890f, /* 2300 */
	1.0000f, 0.6926f, 0.4034f, /* 2400 */
	1.0000f, 0.7021f, 0.4178f, /* 2500 */
	1.0000f, 0.7115f, 0.4323f, /* 2600 */
	1.0000f, 0.7208f, 0.4469f, /* 2700 */
	1.0000f, 0.7299f, 0.4616f, /* 2800 */
	1.0000f, 0.7390f, 0.4763f, /* 2900 */
	1.0000f, 0.7479f, 0.4910f, /* 3000 */
	1.0000f, 0.7568f, 0.5058f, /* 3100 */
	1.0000f, 0.7655f, 0.5206f, /* 3200 */
	1.0000f, 0.7741f, 0.5354f, /* 3300 */
	1.0000f, 0.7826f, 0.5503f, /* 3400 */
	1.0000f, 0.7910f, 0.5652f, /* 3500 */
	1.0000f, 0.7993f, 0.5801f, /* 3600 */
	1.0000f, 0.8075f, 0.5950f, /* 3700 */
	1.0000f, 0.8156f, 0.6099f, /* 3800 */
	1.0000f, 0.8237f, 0.6247f, /* 3900 */
	1.0000f, 0.8315f, 0.6396f, /* 4000 */
	1.0000f, 0.8393f, 0.6546f, /* 4100 */
	1.0000f, 0.8470f, 0.6694f, /* 4200 */
	1.0000f, 0.8546f, 0.6843f, /* 4300 */
	1.0000f, 0.8622f, 0.6991f, /* 4400 */
	1.0000f, 0.8696f, 0.7138f, /* 4500 */
	1.0000f, 0.8769f, 0.7286f, /* 4600 */
	1.0000f, 0.8841f, 0.7433f, /* 4700 */
	1.0000f, 0.8913f, 0.7581f, /* 4800 */
	1.0000f, 0.8984f, 0.7728f, /* 4900 */
	1.0000f, 0.9053f, 0.7874f, /* 5000 */
	1.0000f, 0.9122f, 0.8019f, /* 5100 */
	1.0000f, 0.9190f, 0.8164f, /* 5200 */
	1.0000f, 0.9258f, 0.8309f, /* 5300 */
	1.0000f, 0.9323f, 0.8454f, /* 5400 */
	1.0000f, 0.9388f, 0.8597f, /* 5500 */
	1.0000f, 0.9453f, 0.8740f, /* 5600 */
	1.0000f, 0.9517f, 0.8882f, /* 5700 */
	1.0000f, 0.9580f, 0.9024f, /* 5800 */
	1.0000f, 0.9642f, 0.9165f, /* 5900 */
	1.0000f, 0.9704f, 0.9306f, /* 6000 */
	1.0000f, 0.9764f, 0.9446f, /* 6100 */
	1.0000f, 0.9824f, 0.9586f, /* 6200 */
	1.0000f, 0.9884f, 0.9724f, /* 6300 */
	1.0000f, 0.9942f, 0.9862f, /* 6400 */
	1.0000f, 1.0000f, 1.0000f, /* 6500 */
	0.9867f, 0.9923f, 1.0000f, /* 6600 */
	0.9736f, 0.9847f, 1.0000f, /* 6700 */
	0.9610f, 0.9773f, 1.0000f, /* 6800 */
	0.9488f, 0.9701f, 1.0000f, /* 6900 */
	0.9369f, 0.9630f, 1.0000f, /* 7000 */
	0.9255f, 0.9561f, 1.0000f, /* 7100 */
	0.9142f, 0.9493f, 1.0000f, /* 7200 */
	0.9034f, 0.9428f, 1.0000f, /* 7300 */
	0.8928f, 0.9365f, 1.0000f, /* 7400 */
	0.8826f, 0.9303f, 1.0000f, /* 7500 */
	0.8727f, 0.9242f, 1.0000f, /* 7600 */
	0.8629f, 0.9182f, 1.0000f, /* 7700 */
	0.8537f, 0.9125f, 1.0000f, /* 7800 */
	0.8445f, 0.9068f, 1.0000f, /* 7900 */
	0.8357f, 0.9013f, 1.0000f, /* 8000 */
	0.8271f, 0.8960f, 1.0000f, /* 8100 */
	0.8186f, 0.8907f, 1.0000f, /* 8200 */
	0.8105f, 0.8856f, 1.0000f, /* 8300 */
	0.8025f, 0.8805f, 1.0000f, /* 8400 */
	0.7947f, 0.8757f, 1.0000f, /* 8500 */
	0.7871f, 0.8708f, 1.0000f, /* 8600 */
	0.7798f, 0.8661f, 1.0000f, /* 8700 */
	0.7726f, 0.8615f, 1.0000f, /* 8800 */
	0.7656f, 0.8570f, 1.0000f, /* 8900 */
	0.7588f, 0.8526f, 1.0000f, /* 9000 */
	0.7521f, 0.8483f, 1.0000f, /* 9100 */
	0.7456f, 0.8441f, 1.0000f, /* 9200 */
	0.7393f, 0.8400f, 1.0000f, /* 9300 */
	0.7331f, 0.8359f, 1.0000f, /* 9400 */
	0.7271f, 0.8320f, 1.0000f, /* 9500 */
	0.7212f, 0.8281f, 1.0000f, /* 9600 */
	0.7155f, 0.8243f, 1.0000f, /* 9700 */
	0.7098f, 0.8206f, 1.0000f, /* 9800 */
	0.7043f, 0.8169f, 1.0000f, /* 9900 */
	0.6989f, 0.8133f, 1.0000f, /* 10000 */
};
#else
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
	1.0000f, 0.7826f, 0.5503f, /* 4700K ~= 3400K in f.lux*/
	1.0000f, 0.7970f, 0.5760f, /* 4800K */
	1.0000f, 0.8111f, 0.6016f, /* 4900K */
	1.0000f, 0.8250f, 0.6272f, /* 5000K */
	1.0000f, 0.8384f, 0.6529f, /* 5100K */
	1.0000f, 0.8517f, 0.6785f, /* 5200K */
	1.0000f, 0.8647f, 0.7040f, /* 5300K */
	1.0000f, 0.8773f, 0.7294f, /* 5400K */
	1.0000f, 0.8897f, 0.7548f, /* 5500K */
	1.0000f, 0.9019f, 0.7801f, /* 5600K */
	1.0000f, 0.9137f, 0.8051f, /* 5700K */
	1.0000f, 0.9254f, 0.8301f, /* 5800K */
	1.0000f, 0.9367f, 0.8550f, /* 5900K */
	1.0000f, 0.9478f, 0.8795f, /* 6000K */
	1.0000f, 0.9587f, 0.9040f, /* 6100K */
	1.0000f, 0.9694f, 0.9283f, /* 6200K */
	1.0000f, 0.9798f, 0.9524f, /* 6300K */
	1.0000f, 0.9900f, 0.9763f, /* 6400K */
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
#endif

#ifdef _WIN32
static int exiting=0;
/* Signal handler for exit signals */
BOOL CtrlHandler( DWORD fdwCtrlType ){
	switch( fdwCtrlType ){
    case CTRL_C_EVENT:
      LOG(LOGINFO,_("\nCtrl-C event\n"));
	  exiting=1;
      return( TRUE );
    // CTRL-CLOSE: confirm that the user wants to exit. 
    case CTRL_CLOSE_EVENT:
      LOG(LOGINFO,_("Ctrl-Close event\n"));
	  exiting=1;
      return( TRUE ); 
    // Pass other signals to the next handler.
    case CTRL_BREAK_EVENT:
      LOG(LOGINFO,_("Ctrl-Break event\n"));
      return FALSE; 
    case CTRL_LOGOFF_EVENT:
      LOG(LOGINFO,_("Ctrl-Logoff event\n"));
      return FALSE; 
    case CTRL_SHUTDOWN_EVENT:
      LOG(LOGINFO,_("Ctrl-Shutdown event\n"));
      return FALSE;
    default: 
      return FALSE;
	}
}

/* Register signal handler */
static void sig_register(void){
	if( !SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, TRUE ) )
		LOG(LOGERR,_("Unable to register Control Handler.\n"));
}
#elif defined(HAVE_SYS_SIGNAL_H)
static volatile sig_atomic_t exiting = 0;
/* Signal handler for exit signals */
static void
sigexit(int signo)
{exiting = 1;}

/* Register signal handler */
static void sig_register(void){
	struct sigaction sigact;
	sigset_t sigset;
	sigemptyset(&sigset);

	/* Install signal handler for INT and TERM signals */
	sigact.sa_handler = sigexit;
	sigact.sa_mask = sigset;
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
}
#else /* ! HAVE_SYS_SIGNAL_H */
static int exiting = 0;
static void sig_register(void)
{return;}
#endif /* ! HAVE_SYS_SIGNAL_H */

static void
interpolate_color(float a, const float *c1, const float *c2, float *c)
{
	c[0] = (1.0f-a)*c1[0] + a*c2[0];
	c[1] = (1.0f-a)*c1[1] + a*c2[1];
	c[2] = (1.0f-a)*c1[2] + a*c2[2];
}

void
colorramp_fill(uint16_t *gamma_r, uint16_t *gamma_g, uint16_t *gamma_b,
	       int size, int temp, gamma_s gamma)
{
	int i;
	/* Calculate white point */
	float white_point[3];
	float alpha = (temp % 100) / 100.0f;
	int temp_index = ((temp - 1000) / 100)*3;
	interpolate_color(alpha, &blackbody_color[temp_index],
			  &blackbody_color[temp_index+3], white_point);

	for (i = 0; i < size; i++) {
		gamma_r[i] = (uint16_t)(pow((float)i/size, 1.0f/gamma.r) *
			UINT16_MAX * white_point[0]);
		gamma_g[i] = (uint16_t)(pow((float)i/size, 1.0f/gamma.g) *
			UINT16_MAX * white_point[1]);
		gamma_b[i] = (uint16_t)(pow((float)i/size, 1.0f/gamma.b) *
			UINT16_MAX * white_point[2]);
		//printf("Index %5d: %d %d %d\n",i,gamma_r[i],gamma_g[i],gamma_b[i]);
	}
}

/* Initialize gamma adjustment method. If method is negative
   try all methods until one that works is found. */
gamma_method_t gamma_init_method(int screen_num, int crtc_num,
		gamma_method_t method)
{
	int r;
	gamma_method_t validmethod=GAMMA_METHOD_NONE;
#ifdef ENABLE_RANDR
	if (method = 0 || method == GAMMA_METHOD_RANDR) {
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
	if (method = 0 || method == GAMMA_METHOD_VIDMODE) {
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
	if (method = 0 || method == GAMMA_METHOD_WINGDI) {
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
	return -1;
}

/* Calculate color temperature for the specified solar elevation. */
int calculate_temp(float elevation, int temp_day, int temp_night)
{
	int temp = 0;
	if (elevation < TRANSITION_LOW) {
		temp = temp_night;
		LOG(LOGINFO,_("Period: Night\n"));
	} else if (elevation < TRANSITION_HIGH) {
		/* Transition period: interpolate */
		float a = (TRANSITION_LOW - elevation) /
			(TRANSITION_LOW - TRANSITION_HIGH);
		temp = (int)((1.0f-a)*(float)temp_night + a*(float)temp_day);
		LOG(LOGINFO,_("Period: Transition (%.2f%% day)\n"), a*100);
	} else {
		temp = temp_day;
		LOG(LOGINFO,_("Period: Daytime\n"));
	}

	return temp;
}

/* Change gamma and exit. */
int do_oneshot(gamma_method_t method,float lat,float lon,
		gamma_s gamma, int temp_day, int temp_night)
{
	double now;
	float elevation;
	int r,temp;

	r = systemtime_get_time(&now);
	if (r < 0) {
		LOG(LOGERR,_("Unable to read system time.\n"));
		gamma_state_free(method);
		return -1;
	}

	/* Current angular elevation of the sun */
	elevation = (float)solar_elevation(now, lat, lon);

	/* TRANSLATORS: Append degree symbol if possible. */
	LOG(LOGINFO,_("Solar elevation: %f\n"),elevation);

	/* Use elevation of sun to set color temperature */
	temp = calculate_temp(elevation, temp_day, temp_night);

	LOG(LOGINFO,_("Color temperature: %uK\n"), temp);

	gamma_state_get_temperature(method);
	/* Adjust temperature */
	r = gamma_state_set_temperature(method, temp, gamma);
	if (r < 0) {
		LOG(LOGERR,_("Temperature adjustment failed.\n"));
		gamma_state_free(method);
		return -1;
	}
	gamma_state_free(method);
	return 0;
}

/* Change gamma continuously until break signal. */
int do_continous(gamma_method_t method,float lat,float lon,
		gamma_s gamma, int temp_day, int temp_night,int trans_speed)
{
	int r;
	/* Transition state */
	double short_trans_end = 0;
	int short_trans = 0;
	int short_trans_done = 0;

	/* Make an initial transition from 6500K */
	int short_trans_create = 1;
	int short_trans_begin = 1;
	int short_trans_len = 10;

	/* Amount of adjustment to apply. At zero the color
	   temperature will be exactly as calculated, and at one it
	   will be exactly 6500K. */
	float adjustment_alpha = 0.0;

	int done = 0;
	int disabled = 0;
	double now;
	float elevation;
	int temp;

	sig_register();

	/* Continously adjust color temperature */
	while (1) {
		/* Check to see if exit signal was caught */
		if (exiting) {
			if (done) {
				/* On second signal stop the
				   ongoing transition */
				short_trans = 0;
			} else {
				if (!disabled) {
					/* Make a short transition
					   back to 6500K */
					short_trans_create = 1;
					short_trans_begin = 0;
					short_trans_len = 2;
					adjustment_alpha = 1.0;
				}

				done = 1;
			}
			exiting = 0;
		}

		/* Read timestamp */
		r = systemtime_get_time(&now);
		if (r < 0) {
			LOG(LOGERR,_("Unable to read system time.\n"));
			gamma_state_free(method);
			return -1;
		}

		/* Set up a new transition */
		if (short_trans_create) {
			//if (opts->transition) {
				short_trans_end = now;
				short_trans_end += short_trans_len;
				short_trans = 1;
				short_trans_create = 0;
			//} else {
			//	short_trans_done = 1;
			//}
		}

		/* Current angular elevation of the sun */
		elevation = (float)solar_elevation(now, lat, lon);

		/* Use elevation of sun to set color temperature */
		temp = calculate_temp(elevation, temp_day,temp_night);

		/* Ongoing short transition */
		if (short_trans) {
			double start = now;
			double end = short_trans_end;

			if (start > end) {
				/* Transisiton done */
				short_trans = 0;
				short_trans_done = 1;
			}

			/* Calculate alpha */
			adjustment_alpha = (float)(end - start) /
				(float)short_trans_len;
			if (!short_trans_begin) {
				adjustment_alpha =
					1.0f - adjustment_alpha;
			}

			/* Clamp alpha value */
			adjustment_alpha =
				MAX(0.0f, MIN(adjustment_alpha, 1.0f));
		}

		/* Handle end of transition */
		if (short_trans_done) {
			if (disabled) {
				/* Restore saved gamma ramps */
				gamma_state_restore(method);
			}
			short_trans_done = 0;
		}

		/* Interpolate between 6500K and calculated
		   temperature */
		temp = (int)(adjustment_alpha*6500 +
			(1.0f-adjustment_alpha)*temp);

		/* Quit loop when done */
		if (done && !short_trans) break;

		LOG(LOGINFO,_("Color temperature: %dK\n"), temp);

		/* Adjust temperature */
		if (!disabled || short_trans) {
			r = gamma_state_set_temperature(method,
							temp, gamma);
			if (r < 0) {
				LOG(LOGERR,_("Temperature adjustment"
					" failed.\n"));
				gamma_state_free(method);
				return -1;
			}
		}

		/* Sleep for a while */
#ifndef _WIN32
		if (short_trans) usleep(100000);
		else usleep(5000000);
#else /* ! _WIN32 */
		if (short_trans) Sleep(100);
		else Sleep(5000);
#endif /* ! _WIN32 */
	}

	/* Restore saved gamma ramps */
	gamma_state_restore(method);
	return 0;
}

