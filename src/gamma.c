#include "common.h"
#include "gamma.h"
#include "options.h"
#include "solar.h"
#include "systemtime.h"

#if !(defined(ENABLE_RANDR) ||			\
      defined(ENABLE_VIDMODE) ||		\
      defined(ENABLE_WINGDI))
# error "At least one of RANDR, VidMode or WinGDI must be enabled."
#endif
#include "backends/randr.h"
#include "backends/vidmode.h"
#include "backends/w32gdi.h"

#include "gamma_vals.h"

/* Angular elevation of the sun at which the color temperature
   transition period starts and ends (in degress).
   Transition during twilight, and while the sun is lower than
   3.0 degrees above the horizon. */
#define TRANSITION_LOW     SOLAR_CIVIL_TWILIGHT_ELEV
#define TRANSITION_HIGH    3.0f

static gamma_method_s methods[GAMMA_METHOD_MAX];
static gamma_s default_gam = {DEFAULT_GAMMA,DEFAULT_GAMMA,DEFAULT_GAMMA};
static gamma_method_t active_method=GAMMA_METHOD_NONE;
static gamma_ramp_s ramp = {NULL,NULL,NULL,NULL,0};

// Interpolates between two RGB colors
static void gamma_interp_color(float a,
		const float *c1, const float *c2, float *c)
{
	c[0] = (1.0f-a)*c1[0] + a*c2[0];
	c[1] = (1.0f-a)*c1[1] + a*c2[1];
	c[2] = (1.0f-a)*c1[2] + a*c2[2];
}

// Frees gamma ramps
static int gamma_free_ramps(void){
	if( ramp.all ){
		LOG(LOGINFO,_("Freeing previous ramp"));
		free(ramp.all);
		ramp.all = NULL;
		ramp.r = NULL;
		ramp.g = NULL;
		ramp.b = NULL;
		ramp.size = 0;
	}
	return RET_FUN_SUCCESS;
}

// Re-allocates ramps if needed
gamma_ramp_s gamma_get_ramps(int size){
	if( ramp.size != size ){
		LOG(LOGINFO,_("New ramp size requested, allocating new ramps"));
		gamma_free_ramps();
		ramp.all = (uint16_t*)malloc(sizeof(uint16_t)*3*size);
		if( !ramp.all ){
			LOG(LOGERR,_("Unable to allocate new gamma ramps."));
			return ramp;
		}
		ramp.r = ramp.all;
		ramp.g = ramp.r+size;
		ramp.b = ramp.g+size;
		ramp.size = size;
	}
	return ramp;
}

// Fill gamma ramp according to current parameters
gamma_ramp_s gamma_ramp_fill(int size, int temp)
{
	int i;
	/* Calculate white point */
	float white_point[3];
	float alpha = (temp % 100) / 100.0f;
	int temp_index = ((temp - 1000) / 100)*3;
	float brightness = opt_get_brightness();
	gamma_s tweak = opt_get_gamma();
	gamma_ramp_s curr_ramp = gamma_get_ramps(size);

	gamma_interp_color(alpha, &blackbody_color[temp_index],
			  &blackbody_color[temp_index+3], white_point);

	if(!curr_ramp.size)
		return curr_ramp;
	for (i = 0; i < size; i++) {
		curr_ramp.r[i] = (uint16_t)(brightness*
				(pow((float)i/size,1.0f/tweak.r)*
				 UINT16_MAX * white_point[0]));
		curr_ramp.g[i] = (uint16_t)(brightness*
				(pow((float)i/size,1.0f/tweak.g)*
				 UINT16_MAX * white_point[1]));
		curr_ramp.b[i] = (uint16_t)(brightness*
				(pow((float)i/size,1.0f/tweak.b)*
				 UINT16_MAX * white_point[2]));
	}
	return curr_ramp;
}

char *gamma_get_active_method_name(void){
	if( methods[active_method].name )
		return methods[active_method].name;
	else
		return "None";
}

char *gamma_get_method_name(gamma_method_t method){
	if( methods[method].name )
		return methods[method].name;
	else
		return "None";
}

int gamma_find_temp(float ratio){
	int i;
	int gam_val_size=SIZEOF(blackbody_color);
	float prev_ratio,curr_ratio;
	LOG(LOGVERBOSE,_("R/B Ratio: %f"),ratio);
	for(i=0; i<gam_val_size; ++i){
		curr_ratio = (float)blackbody_color[i*3]/(float)blackbody_color[i*3+2];
		if( curr_ratio <= ratio ){
			// Interpolate color based on ratio
			int color = (ratio-curr_ratio)/(prev_ratio-curr_ratio)*100
				+i*100+1000;
			LOG(LOGVERBOSE,_("Current col:%d"),color);
			return color;
		}
		prev_ratio = curr_ratio;
	}
	LOG(LOGERR,_("Unable to find color temperature"));
	return RET_FUN_FAILED;
}

/* Looks up gamma method by name */
gamma_method_t gamma_lookup_method(char *name){
	gamma_method_t i;
	for( i=0; i<GAMMA_METHOD_MAX; ++i ){
		if( (methods[i].name)
			&& (strcmp(name,methods[i].name)==0) )
			return i;
	}
	return GAMMA_METHOD_NONE;
}

/* Loads all available gamma adjustment methods */
int gamma_load_methods(void){
	int i;
	for ( i=0; i<GAMMA_METHOD_MAX; ++i){
		methods[i].func_init = NULL;
		methods[i].func_end = NULL;
		methods[i].func_set_temp = NULL;
		methods[i].func_get_temp = NULL;
		methods[i].name = NULL;
	}
	methods[GAMMA_METHOD_AUTO].name = "Auto";
#ifdef ENABLE_RANDR
	randr_load_funcs(&methods[GAMMA_METHOD_RANDR]);
#endif
#ifdef ENABLE_VIDMODE
	vidmode_load_funcs(&methods[GAMMA_METHOD_VIDMODE]);
#endif
#ifdef ENABLE_WINGDI
	w32gdi_load_funcs(&methods[GAMMA_METHOD_WINGDI]);
#endif
	return RET_FUN_SUCCESS;
}

/* Initialize gamma adjustment method. */
gamma_method_t gamma_init_method(int screen_num, int crtc_num,
		gamma_method_t method)
{
	gamma_method_t curr=method;
	gamma_method_t trymethod=method;
	gamma_method_t validmethod=GAMMA_METHOD_NONE;

	if( active_method != GAMMA_METHOD_NONE ){
		LOG(LOGERR,_("You must free previous method before init"));
		return GAMMA_METHOD_NONE;
	}
	do{
		if(methods[curr].func_init){
			LOG(LOGINFO,_("Trying %s method"),methods[curr].name);
			if( methods[curr].func_init(screen_num,crtc_num) ){
				validmethod = curr;
				active_method = validmethod;
			}else
				LOG(LOGERR,_("Initialization of %s failed."),
						methods[curr].name);
		}
		++curr;
	}while( (trymethod == GAMMA_METHOD_AUTO)
			&& (curr < GAMMA_METHOD_MAX)
			&& (validmethod == GAMMA_METHOD_NONE) );
	if( validmethod <= GAMMA_METHOD_AUTO ){
		LOG(LOGERR,_("Could not initialize any valid methods"));
		return GAMMA_METHOD_NONE;
	}
	return validmethod;
}

/* Restore saved gamma ramps with the appropriate adjustment method. */
int gamma_state_restore(void)
{
	if( methods[active_method].func_set_temp )
		return methods[active_method].func_set_temp(DEFAULT_DAY_TEMP,default_gam);
	else
		LOG(LOGERR,_("Invalid active method for restoring ramps"));
	return RET_FUN_FAILED;
}

/* Free the state associated with the appropriate adjustment method. */
int gamma_state_free(void)
{
	gamma_free_ramps();
	if( methods[active_method].func_end ){
		if( methods[active_method].func_end() ){
			active_method = GAMMA_METHOD_NONE;
			return RET_FUN_SUCCESS;
		}
	}
	LOG(LOGERR,_("Invalid active method for freeing"));
	return RET_FUN_FAILED;
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
int gamma_state_set_temperature(int temp, gamma_s gamma)
{
	if( methods[active_method].func_set_temp )
		return methods[active_method].func_set_temp(temp,gamma);
	return RET_FUN_FAILED;
}

/* Retrieves temperature with the appropriate adjustment method. */
int gamma_state_get_temperature(void){
	if( methods[active_method].func_get_temp )
		return methods[active_method].func_get_temp();
	return RET_FUN_FAILED;
}

