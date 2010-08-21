#include "common.h"
/*@ignore@*/
#include <X11/Xlib.h>
#include <X11/extensions/xf86vmode.h>
/*@end@*/
#include "gamma.h"
#include "vidmode.h"

/**\brief VidMode state storage */
typedef struct {
	/**\brief Display pointer */
	Display *display;
	/**\brief Screen number in use */
	int screen_num;
	/**\brief Size of the gamma ramps */
	int ramp_size;
	/**\brief Saved ramps */
	uint16_t *saved_ramps;
} vidmode_state_t;

static vidmode_state_t state={NULL,0,0,NULL};

int vidmode_init(int screen_num,int crtc_num)
{
	int major, minor;
	uint16_t *gamma_r;
	uint16_t *gamma_g;
	uint16_t *gamma_b;

	/* Open display */
	state.display = XOpenDisplay(NULL);
	if (state.display == NULL) {
		LOG(LOGERR, _("X request failed: %s\n"),
			"XOpenDisplay");
		return RET_FUN_FAILED;
	}

	if (screen_num < 0) screen_num = DefaultScreen(state.display);
	state.screen_num = screen_num;

	/* Query extension version */
	if( !XF86VidModeQueryVersion(state.display, &major, &minor) ){
		LOG(LOGERR, _("X request failed: %s\n"),
			"XF86VidModeQueryVersion");
		XCloseDisplay(state.display);
		return RET_FUN_FAILED;
	}

	/* Request size of gamma ramps */
	if( !XF86VidModeGetGammaRampSize(state.display, state.screen_num,
					&state.ramp_size) ){
		LOG(LOGERR, _("X request failed: %s\n"),
			"XF86VidModeGetGammaRampSize");
		XCloseDisplay(state.display);
		return RET_FUN_FAILED;
	}

	if (state.ramp_size == 0) {
		LOG(LOGERR, _("Gamma ramp size too small: %i\n"),
			state.ramp_size);
		XCloseDisplay(state.display);
		return RET_FUN_FAILED;
	}

	/* Allocate space for saved gamma ramps */
	state.saved_ramps = malloc(3*state.ramp_size*sizeof(uint16_t));
	if (state.saved_ramps == NULL) {
		perror("malloc");
		XCloseDisplay(state.display);
		return RET_FUN_FAILED;
	}

	gamma_r = &state.saved_ramps[0*state.ramp_size];
	gamma_g = &state.saved_ramps[1*state.ramp_size];
	gamma_b = &state.saved_ramps[2*state.ramp_size];

	/* Save current gamma ramps so we can restore them at program exit. */
	if( !XF86VidModeGetGammaRamp(state.display, state.screen_num,
				    state.ramp_size, gamma_r, gamma_g,
				    gamma_b) ){
		LOG(LOGERR, _("X request failed: %s\n"),
			"XF86VidModeGetGammaRamp");
		XCloseDisplay(state.display);
		return RET_FUN_FAILED;
	}

	return RET_FUN_SUCCESS;
}

int vidmode_free(void)
{
	/* Free saved ramps */
	free(state.saved_ramps);

	/* Close display connection */
	XCloseDisplay(state.display);
	return RET_FUN_SUCCESS;
}

void vidmode_restore(void)
{
	uint16_t *gamma_r = &state.saved_ramps[0*state.ramp_size];
	uint16_t *gamma_g = &state.saved_ramps[1*state.ramp_size];
	uint16_t *gamma_b = &state.saved_ramps[2*state.ramp_size];

	/* Restore gamma ramps */
	if( !XF86VidModeSetGammaRamp(state.display, state.screen_num,
					state.ramp_size, gamma_r, gamma_g,
					gamma_b) ){
		LOG(LOGERR, _("X request failed: %s\n"),
			"XF86VidModeSetGammaRamp");
	}
}

int vidmode_set_temperature(int temp, gamma_s gamma)
{
	/* Create new gamma ramps */
	gamma_ramp_s ramp;

	ramp = gamma_ramp_fill(state.ramp_size,temp);
	if( !ramp.size )
		return RET_FUN_FAILED;

	/* Set new gamma ramps */
	if( !XF86VidModeSetGammaRamp(state.display, state.screen_num,
				    ramp.size, ramp.r, ramp.g,
				    ramp.b)){
		LOG(LOGERR, _("X request failed: %s\n"),
			"XF86VidModeSetGammaRamp");
		return RET_FUN_FAILED;
	}
	return RET_FUN_SUCCESS;
}

int vidmode_get_temperature(void){
	uint16_t *gamma_r = malloc(state.ramp_size*sizeof(uint16_t));
	uint16_t *gamma_g = malloc(state.ramp_size*sizeof(uint16_t));
	uint16_t *gamma_b = malloc(state.ramp_size*sizeof(uint16_t));
	if( !(gamma_r && gamma_g && gamma_b) ){
		perror("malloc");
		return RET_FUN_FAILED;
	}

	if( !XF86VidModeGetGammaRamp(state.display,state.screen_num,
				state.ramp_size,
				gamma_r,gamma_g,gamma_b) ){
		LOG(LOGERR,_("X request failed"));
		free(gamma_r);
		free(gamma_g);
		free(gamma_b);
		return RET_FUN_FAILED;
	}else{
		uint16_t gamma_r_end = gamma_r[state.ramp_size-1];
		uint16_t gamma_b_end = gamma_b[state.ramp_size-1];
		float rb_ratio = (float)gamma_r_end/(float)gamma_b_end;

		LOG(LOGVERBOSE,_("Red end: %uK, Blue end: %uK"),
				gamma_r_end,gamma_b_end);
		free(gamma_r);
		free(gamma_g);
		free(gamma_b);
		return gamma_find_temp(rb_ratio);
	}
}

int vidmode_load_funcs(gamma_method_s *method){
	method->func_init = &vidmode_init;
	method->func_end = &vidmode_free;
	method->func_set_temp = &vidmode_set_temperature;
	method->func_get_temp = &vidmode_get_temperature;
	method->name = "VidMode";
	return RET_FUN_SUCCESS;
}
