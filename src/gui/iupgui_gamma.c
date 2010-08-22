#include "common.h"
#include "options.h"
#include "gamma.h"
#include "iupgui.h"
#include "iupgui_main.h"

/*@null@*/ static Ihandle *timer_gamma_check=NULL;
/*@null@*/ static Ihandle *timer_gamma_transition=NULL;

static int curr_temp=1000;
static int target_temp=1000;
static int timers_disabled = 0;

// Changes temperature
static int _gamma_transition(/*@unused@*/ Ihandle *ih){
	int transpeed = opt_get_trans_speed();
	if( curr_temp > target_temp ){
		curr_temp -= transpeed/10;
		if( curr_temp < target_temp ){
			(void)guigamma_set_temp(target_temp);
			IupSetAttribute(timer_gamma_transition,"RUN","NO");
			IupSetAttribute(timer_gamma_check,"RUN","YES");
			return IUP_DEFAULT;
		}
	}else{
		curr_temp += transpeed/10;
		if( curr_temp > target_temp ){
			(void)guigamma_set_temp(target_temp);
			IupSetAttribute(timer_gamma_transition,"RUN","NO");
			IupSetAttribute(timer_gamma_check,"RUN","YES");
			return IUP_DEFAULT;
		}
	}

	LOG(LOGVERBOSE,_("Transition color: %uK"),curr_temp);
	if( !gamma_state_set_temperature(
				curr_temp,opt_get_gamma()) ){
		LOG(LOGERR,_("Temperature adjustment failed."));
		IupSetAttribute(timer_gamma_transition,"RUN","NO");
		return IUP_DEFAULT;
	}
	guimain_update_info();
	return IUP_DEFAULT;
}

// Returns current temperature as known by GUI
int guigamma_get_temp(void){
	return curr_temp;
}

// Sets the current temperature in GUI
int guigamma_set_temp(int temp){
	(void)gamma_state_set_temperature(temp,opt_get_gamma());
	curr_temp = temp;
	return RET_FUN_SUCCESS;
}

// Check if temperature needs to be corrected
int guigamma_check(/*@unused@*/ Ihandle *ih){

	if( timers_disabled )
		return IUP_DEFAULT;

	target_temp = gamma_calc_curr_target_temp(
			opt_get_lat(),opt_get_lon(),
			opt_get_temp_day(),opt_get_temp_night());
	LOG(LOGINFO,_("Gamma check, current: %d, target: %d"),
			curr_temp,target_temp);
	if( fabs((double)(curr_temp - target_temp)) >= 1.0 ){
		// Disable current timer
		IupSetAttribute(timer_gamma_check,"RUN","NO");
		IupSetAttribute(timer_gamma_transition,"RUN","YES");
	}
	guimain_update_info();
	return IUP_DEFAULT;
}

// Disables gamma timers and checking
void guigamma_disable(void){
	(void)guigamma_set_temp(DEFAULT_DAY_TEMP);
	guimain_update_info();
	IupSetAttribute(timer_gamma_check,"RUN","NO");
	IupSetAttribute(timer_gamma_transition,"RUN","NO");
	timers_disabled = 1;
}

// Enables gamma timers
void guigamma_enable(void){
	IupSetAttribute(timer_gamma_check,"RUN","YES");
	timers_disabled = 0;
}

// Initialize timer to run gamma correction
void guigamma_init_timers(void){
	// Re-check every 5 minute
	timer_gamma_check = IupTimer();
	IupSetfAttribute(timer_gamma_check,"TIME","%d",1000*60*5);
	(void)IupSetCallback(timer_gamma_check,"ACTION_CB",(Icallback)guigamma_check);
	IupSetAttribute(timer_gamma_check,"RUN","YES");

	// Transition step size is 100 ms
	timer_gamma_transition = IupTimer();
	IupSetfAttribute(timer_gamma_transition,"TIME","%d",100);
	(void)IupSetCallback(timer_gamma_transition,"ACTION_CB",(Icallback)_gamma_transition);

	// Make sure gamma is synced up
	curr_temp = gamma_state_get_temperature();
	(void)gamma_state_set_temperature(curr_temp,opt_get_gamma());
	(void)guigamma_check(timer_gamma_check);
}

// Destroys timers
void guigamma_end_timers(void){
	if( timer_gamma_check )
		IupDestroy(timer_gamma_check);

	if( timer_gamma_transition )
		IupDestroy(timer_gamma_transition);

}
