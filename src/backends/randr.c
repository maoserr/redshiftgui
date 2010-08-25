#include "common.h"
/*@ignore@*/
#include <xcb/xcb.h>
#include <xcb/randr.h>
/*@end@*/
#include "gamma.h"
#include "randr.h"

/**\brief randr storage of crtc state info */
typedef struct {
	/**\brief crtc number */
	xcb_randr_crtc_t crtc;
	/**\brief length of gamma ramps */
	unsigned int ramp_size;
	/**\brief pointer to saved gamma ramps */
	/*@null@*/ uint16_t *saved_ramps;
} randr_crtc_state_t;

/**\brief randr storage of state info */
typedef struct {
	/**\brief xcb connection pointer */
	/*@null@*/ xcb_connection_t *conn;
	/**\brief xcb screen pointer */
	/*@null@*/ xcb_screen_t *screen;
	/**\brief crtc number */
	int crtc_num;
	/**\brief number of crtc */
	unsigned int crtc_count;
	/**\brief state of crtcs*/
	/*@null@*/ randr_crtc_state_t *crtcs;
} randr_state_t;

#define RANDR_VERSION_MAJOR  1
#define RANDR_VERSION_MINOR  3

static randr_state_t state={NULL,NULL,0,0,NULL};

int randr_init(int screen_num, int crtc_num)
{
	xcb_generic_error_t *error;
	xcb_randr_query_version_cookie_t ver_cookie;
	xcb_randr_query_version_reply_t *ver_reply;
	const xcb_setup_t *setup;
	int i;
	xcb_screen_iterator_t iter;
	xcb_randr_get_screen_resources_current_cookie_t res_cookie;
	xcb_randr_get_screen_resources_current_reply_t *res_reply;
	xcb_randr_crtc_t *crtcs;
	unsigned int ramp_size;
	xcb_randr_get_crtc_gamma_cookie_t gamma_get_cookie;
	xcb_randr_get_crtc_gamma_reply_t *gamma_get_reply;
	uint16_t *gamma_r;
	uint16_t *gamma_g;
	uint16_t *gamma_b;
	
	/* Open X server connection */
	int preferred_screen;
	LOG(LOGINFO,_("Initializing RANDR backend"));

	if( state.conn!=NULL ){
		LOG(LOGERR,_("Connection already established."));
		return RET_FUN_FAILED;
	}
	state.conn = xcb_connect(NULL, &preferred_screen);

	if (screen_num < 0)
		screen_num = preferred_screen;

	/* Query RandR version */
	ver_cookie=xcb_randr_query_version(state.conn,
			RANDR_VERSION_MAJOR,RANDR_VERSION_MINOR);
	ver_reply = xcb_randr_query_version_reply(state.conn,
			ver_cookie, &error);

	if (error) {
		LOG(LOGERR, _("`%s' returned error %d\n"),
			"RANDR Query Version", error->error_code);
		free(ver_reply);
		xcb_disconnect(state.conn);
		return RET_FUN_FAILED;
	}

	if (ver_reply->major_version < RANDR_VERSION_MAJOR ||
	    ver_reply->minor_version < RANDR_VERSION_MINOR) {
		LOG(LOGERR, _("Unsupported RANDR version (%u.%u)\n"),
			ver_reply->major_version, ver_reply->minor_version);
		free(ver_reply);
		xcb_disconnect(state.conn);
		return RET_FUN_FAILED;
	}

	free(ver_reply);

	/* Get screen */
	setup = xcb_get_setup(state.conn);
	iter = xcb_setup_roots_iterator(setup);

	for (i = 0; iter.rem > 0; i++) {
		if (i == screen_num) {
			/*@i1@*/state.screen = iter.data;
			break;
		}
		xcb_screen_next(&iter);
	}

	if (state.screen == NULL) {
		LOG(LOGERR, _("Screen %i could not be found.\n"),
			screen_num);
		xcb_disconnect(state.conn);
		return RET_FUN_FAILED;
	}

	/* Get list of CRTCs for the screen */
	res_cookie = xcb_randr_get_screen_resources_current(state.conn,
					state.screen->root);
	res_reply = xcb_randr_get_screen_resources_current_reply(state.conn,
					res_cookie,
					/*@i1@*/&error);

	if (error) {
		LOG(LOGERR, _("`%s' returned error %d\n"),
			"RANDR Get Screen Resources Current",
			error->error_code);
		free(res_reply);
		xcb_disconnect(state.conn);
		return RET_FUN_FAILED;
	}

	state.crtc_num = crtc_num;
	state.crtc_count = (unsigned int)res_reply->num_crtcs;
	if( state.crtcs!=NULL )
		free(state.crtcs);
	state.crtcs = malloc(state.crtc_count * sizeof(randr_crtc_state_t));
	if (state.crtcs == NULL) {
		perror("malloc");
		free(res_reply);
		xcb_disconnect(state.conn);
		/*@i1@*/return RET_FUN_FAILED;
	}

	crtcs = xcb_randr_get_screen_resources_current_crtcs(res_reply);

	/* Save CRTC identifier in state */
	for (i = 0; i < ((int)state.crtc_count); i++) {
		state.crtcs[i].crtc = crtcs[i];
	}

	free(res_reply);

	/* Save size and gamma ramps of all CRTCs.
	   Current gamma ramps are saved so we can restore them
	   at program exit. */
	for (i = 0; i < ((int)state.crtc_count); i++) {
		/*@i2@*/xcb_randr_crtc_t crtc = state.crtcs[i].crtc;

		/* Request size of gamma ramps */
		xcb_randr_get_crtc_gamma_size_cookie_t gamma_size_cookie =
			xcb_randr_get_crtc_gamma_size(state.conn, crtc);
		xcb_randr_get_crtc_gamma_size_reply_t *gamma_size_reply =
			xcb_randr_get_crtc_gamma_size_reply(state.conn,
							    gamma_size_cookie,
							    /*@i1@*/&error);

		if (error) {
			LOG(LOGERR, _("`%s' returned error %d\n"),
				"RANDR Get CRTC Gamma Size",
				error->error_code);
			free(gamma_size_reply);
			xcb_disconnect(state.conn);
			/*@i1@*/return RET_FUN_FAILED;
		}

		ramp_size = (unsigned int)gamma_size_reply->size;
		state.crtcs[i].ramp_size = ramp_size;

		free(gamma_size_reply);

		if (ramp_size == 0) {
			LOG(LOGERR, _("Gamma ramp size too small: %i\n"),
				ramp_size);
			xcb_disconnect(state.conn);
			/*@i1@*/return RET_FUN_FAILED;
		}

		/* Request current gamma ramps */
		gamma_get_cookie = xcb_randr_get_crtc_gamma(state.conn, crtc);
		gamma_get_reply = xcb_randr_get_crtc_gamma_reply(state.conn,
						       gamma_get_cookie,
						       &error);

		if (error) {
			LOG(LOGERR, _("`%s' returned error %d\n"),
				"RANDR Get CRTC Gamma", error->error_code);
			free(gamma_get_reply);
			xcb_disconnect(state.conn);
			/*@i1@*/return RET_FUN_FAILED;
		}

		gamma_r = xcb_randr_get_crtc_gamma_red(gamma_get_reply);
		gamma_g = xcb_randr_get_crtc_gamma_green(gamma_get_reply);
		gamma_b = xcb_randr_get_crtc_gamma_blue(gamma_get_reply);

		/* Allocate space for saved gamma ramps */
		state.crtcs[i].saved_ramps =
			malloc(3*ramp_size*sizeof(uint16_t));
		if (state.crtcs[i].saved_ramps == NULL) {
			perror("malloc");
			free(gamma_get_reply);
			xcb_disconnect(state.conn);
			/*@i1@*/return RET_FUN_FAILED;
		}

		/* Copy gamma ramps into CRTC state */
		/*@i6@*/memcpy(state.crtcs[i].saved_ramps+0*ramp_size, gamma_r,
		       ramp_size*sizeof(uint16_t));
		memcpy(state.crtcs[i].saved_ramps+1*ramp_size, gamma_g,
		       ramp_size*sizeof(uint16_t));
		memcpy(state.crtcs[i].saved_ramps+2*ramp_size, gamma_b,
		       ramp_size*sizeof(uint16_t));

		free(gamma_get_reply);
	}

	/*@i1@*/return RET_FUN_SUCCESS;
}

void randr_restore(void){
	xcb_generic_error_t *error;
	int i;

	if( (state.conn==NULL)
			||(state.crtcs==NULL) )
		return;

	/* Restore CRTC gamma ramps */
	for (i = 0; i < ((int)state.crtc_count); i++) {
		xcb_randr_crtc_t crtc = state.crtcs[i].crtc;

		uint16_t ramp_size =(uint16_t)state.crtcs[i].ramp_size;
		uint16_t *gamma_r;
		uint16_t *gamma_g;
		uint16_t *gamma_b;
		/* Set gamma ramps */
		xcb_void_cookie_t gamma_set_cookie;

		if( state.crtcs[i].saved_ramps==NULL )
			return;

		gamma_r = &state.crtcs[i].saved_ramps[0*ramp_size];
		gamma_g = &state.crtcs[i].saved_ramps[1*ramp_size];
		gamma_b = &state.crtcs[i].saved_ramps[2*ramp_size];

		gamma_set_cookie = xcb_randr_set_crtc_gamma_checked(
				state.conn, crtc,
				ramp_size, gamma_r,
				gamma_g, gamma_b);

		error = xcb_request_check(state.conn, gamma_set_cookie);

		if (error) {
			LOG(LOGERR, _("`%s' returned error %d\n"),
				"RANDR Set CRTC Gamma", error->error_code);
			LOG(LOGERR, _("Unable to restore CRTC %i\n"), i);
		}
	}
}

int randr_free(void){
	int i;

	LOG(LOGVERBOSE,_("Freeing Randr specific memory"));
	if( state.crtcs==NULL )
		return RET_FUN_FAILED;

	/* Free CRTC state */
	for( i=0; i<state.crtc_count; ++i ){
		if( state.crtcs[i].saved_ramps!=NULL ){
			LOG(LOGVERBOSE,_("Freeing Randr CRTC %d"),i);
			free(state.crtcs[i].saved_ramps);
			state.crtcs[i].saved_ramps=NULL;
		}
	}
	free(state.crtcs);
	state.crtcs=NULL;

	/* Close connection */
	if( state.conn!=NULL )
		xcb_disconnect(state.conn);

	LOG(LOGVERBOSE,_("Randr memory freed successfully."));
	return RET_FUN_SUCCESS;
}

static int randr_set_temperature_for_crtc(int crtc_num, int temp,
			       /*@unused@*/ gamma_s gamma)
{
	xcb_generic_error_t *error;
	gamma_ramp_s ramp;
	unsigned int ramp_size;
	xcb_randr_crtc_t crtc;
	xcb_void_cookie_t gamma_set_cookie;

	if ( (crtc_num>=((int)state.crtc_count))
			||(crtc_num<0)
			||(state.crtcs==NULL) ) {
		LOG(LOGERR, _("CRTC %d does not exist. "),
			state.crtc_num);
		if (state.crtc_count > 1) {
			LOG(LOGERR, _("Valid CRTCs are [0-%d].\n"),
				state.crtc_count-1);
		} else {
			LOG(LOGERR, _("Only CRTC 0 exists.\n"));
		}

		return RET_FUN_FAILED;
	}

	crtc = state.crtcs[crtc_num].crtc;
	ramp_size = state.crtcs[crtc_num].ramp_size;

	ramp = gamma_ramp_fill((int)ramp_size,temp);
	if( (!ramp.size)
			||(ramp.r==NULL)
			||(ramp.g==NULL)
			||(ramp.b==NULL) )
		return RET_FUN_FAILED;
	if( state.conn==NULL ){
		LOG(LOGERR,_("No connection available"));
		return RET_FUN_FAILED;
	}

	/* Set new gamma ramps */
	gamma_set_cookie = xcb_randr_set_crtc_gamma_checked(state.conn, crtc,
						 (uint16_t)ramp_size, ramp.r,
						 ramp.g, ramp.b);
	error = xcb_request_check(state.conn, gamma_set_cookie);

	if (error) {
		LOG(LOGERR, _("`%s' returned error %d"),
			"RANDR Set CRTC Gamma", error->error_code);
		return RET_FUN_FAILED;
	}
	LOG(LOGVERBOSE,_("Set gamma[CRTC %d], end points: (%d,%d)"),
			crtc_num,ramp.r[ramp_size-1],ramp.b[ramp_size-1]);

	return RET_FUN_SUCCESS;
}

int randr_set_temperature(int temp, gamma_s gamma){
	/* If no CRTC number has been specified,
	   set temperature on all CRTCs. */
	if (state.crtc_num < 0) {
		int i;
		for (i = 0; i < ((int)state.crtc_count); i++) {
			if(!randr_set_temperature_for_crtc(i,
							   temp, gamma))
				return RET_FUN_FAILED;
		}
	} else {
		return randr_set_temperature_for_crtc(state.crtc_num,
						      temp, gamma);
	}

	return RET_FUN_SUCCESS;
}

int randr_get_temperature(void){
	randr_crtc_state_t crtc;
	xcb_generic_error_t *error;
	xcb_randr_get_crtc_gamma_cookie_t gamma_get_cookie;
	xcb_randr_get_crtc_gamma_reply_t* gamma_get_reply;

	if( state.crtcs==NULL ){
		LOG(LOGERR,_("CRTCs not defined."));
		return RET_FUN_FAILED;
	}
	if( state.conn==NULL ){
		LOG(LOGERR,_("Connection not established."));
		return RET_FUN_FAILED;
	}

	crtc = state.crtc_num<0 ?
		state.crtcs[0] :
		state.crtcs[state.crtc_num];
	gamma_get_cookie= xcb_randr_get_crtc_gamma(state.conn,crtc.crtc);
	gamma_get_reply = xcb_randr_get_crtc_gamma_reply(state.conn,
			gamma_get_cookie, &error);

	if(error){
		LOG(LOGERR,_("`%s' returned error %d"),"RANDR Get CRTC Gamma",
				error->error_code);
		free(gamma_get_reply);
		/*@i2@*/return RET_FUN_FAILED;
	}else{
		uint16_t *gamma_r,*gamma_b;
		int gamma_r_length,gamma_b_length;
		uint16_t gamma_r_end,gamma_b_end;
		float rb_ratio;
	
		gamma_r = xcb_randr_get_crtc_gamma_red(gamma_get_reply);
		gamma_b = xcb_randr_get_crtc_gamma_blue(gamma_get_reply);
		gamma_r_length = xcb_randr_get_crtc_gamma_red_length(gamma_get_reply);
		gamma_b_length = xcb_randr_get_crtc_gamma_blue_length(gamma_get_reply);
		gamma_r_end = gamma_r[gamma_r_length-1];
		gamma_b_end = gamma_b[gamma_b_length-1];
		LOG(LOGVERBOSE,_("Gamma end points: (%d,%d)"),
				gamma_r_end,gamma_b_end);
		rb_ratio = ((float)gamma_r_end)/((float)gamma_b_end);
		free(gamma_get_reply);
		/*@i2@*/return gamma_find_temp(rb_ratio);
	}
}

int randr_load_funcs(gamma_method_s *method){
	method->func_init = &randr_init;
	method->func_end = &randr_free;
	method->func_set_temp = &randr_set_temperature;
	method->func_get_temp = &randr_get_temperature;
	method->name = "RANDR";
	return RET_FUN_SUCCESS;
}
