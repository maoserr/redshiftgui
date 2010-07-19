#include "common.h"
#include "iup.h"
#include "options.h"
#include "gamma.h"
#include "iupgui_main.h"

static Ihandle *timer_gamma_check=NULL;
static Ihandle *timer_gamma_transition=NULL;

static int curr_temp=1000;
static int target_temp=1000;
static int transpeed;
static int timers_disabled = 0;

// Changes temperature
static int _gamma_transition(Ihandle *ih){
	if( curr_temp > target_temp ){
		curr_temp -= transpeed/10;
		if( curr_temp < target_temp ){
			curr_temp += transpeed/10;
			IupSetAttribute(timer_gamma_transition,"RUN","NO");
			IupSetAttribute(timer_gamma_check,"RUN","YES");
			return IUP_DEFAULT;
		}
	}else{
		curr_temp += transpeed/10;
		if( curr_temp > target_temp ){
			curr_temp -= transpeed/10;
			IupSetAttribute(timer_gamma_transition,"RUN","NO");
			IupSetAttribute(timer_gamma_check,"RUN","YES");
			return IUP_DEFAULT;
		}
	}

	LOG(LOGVERBOSE,_("Transition color: %uK"),curr_temp);
	if( !gamma_state_set_temperature(opt_get_method(),
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
	gamma_state_set_temperature(opt_get_method(),temp,opt_get_gamma());
	curr_temp = temp;
	return RET_FUN_SUCCESS;
}

// Check if temperature needs to be corrected
int guigamma_check(Ihandle *ih){

	if( timers_disabled )
		return IUP_DEFAULT;

	target_temp = gamma_calc_curr_target_temp(
			opt_get_lat(),opt_get_lon(),
			opt_get_temp_day(),opt_get_temp_night());
	LOG(LOGINFO,_("Gamma check, current: %d, target: %d"),
			curr_temp,target_temp);
	if( fabs(curr_temp - target_temp) >= 100 ){
		// Disable current timer
		IupSetAttribute(timer_gamma_check,"RUN","NO");
		IupSetAttribute(timer_gamma_transition,"RUN","YES");
	}
	guimain_update_info();
	return IUP_DEFAULT;
}

// Disables gamma timers and checking
void guigamma_disable(void){
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
	transpeed = opt_get_trans_speed();

	// Re-check every 5 minute
	timer_gamma_check = IupTimer();
	IupSetfAttribute(timer_gamma_check,"TIME","%d",1000*60*5);
	IupSetCallback(timer_gamma_check,"ACTION_CB",(Icallback)guigamma_check);
	IupSetAttribute(timer_gamma_check,"RUN","YES");

	// Transition step size is 100 ms
	timer_gamma_transition = IupTimer();
	IupSetfAttribute(timer_gamma_transition,"TIME","%d",100);
	IupSetCallback(timer_gamma_transition,"ACTION_CB",(Icallback)_gamma_transition);

	// Make sure gamma is synced up
	curr_temp = gamma_state_get_temperature(opt_get_method());
	gamma_state_set_temperature(opt_get_method(),curr_temp,opt_get_gamma());
	guigamma_check(timer_gamma_check);
}

