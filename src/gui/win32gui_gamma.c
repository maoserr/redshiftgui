#include "common.h"
#include "gamma.h"
#include "options.h"
#include "gui/win32gui.h"
#include "gui/win32gui_gamma.h"

static UINT timer_gamma_check=(UINT)NULL;
static UINT timer_gamma_transition=(UINT)NULL;

#define IDT_GAMMA_CHECK 2000
#define IDT_GAMMA_TRANS 2001

static int curr_temp=1000;
static int target_temp=1000;
static int timers_disabled = 0;

static void _gamma_toggle_timer_trans(int onoff);
static void _gamma_toggle_timer_check(int onoff);

// Changes temperature
static void _gamma_transition(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime){
	int transpeed = opt_get_trans_speed();
	if( curr_temp > target_temp ){
		curr_temp -= transpeed/10;
		if( curr_temp < target_temp ){
			(void)guigamma_set_temp(target_temp);
			_gamma_toggle_timer_trans(0);
			_gamma_toggle_timer_check(1);
			return;
		}
	}else{
		curr_temp += transpeed/10;
		if( curr_temp > target_temp ){
			(void)guigamma_set_temp(target_temp);
			_gamma_toggle_timer_trans(0);
			_gamma_toggle_timer_check(1);
			return;
		}
	}

	LOG(LOGVERBOSE,_("Transition color: %dK"),curr_temp);
	if( !gamma_state_set_temperature(
				curr_temp,opt_get_gamma()) ){
		LOG(LOGERR,_("Temperature adjustment failed (Target %d."),
			curr_temp);
	}
	guimain_update_info();
	return;
}


// Toggles transition timer
static void _gamma_toggle_timer_trans(int onoff){
	if(onoff){
		// Transition step size is 100 ms
		timer_gamma_transition = SetTimer(NULL,IDT_GAMMA_TRANS,100,(TIMERPROC)_gamma_transition);
	}else{
		if(timer_gamma_transition){
			KillTimer(NULL,timer_gamma_transition);
			timer_gamma_transition = (UINT) NULL;
		}
	}
}

// Toggles check timer
static void _gamma_toggle_timer_check(int onoff){
	if(onoff){
		// Re-check every 5 minute
		timer_gamma_check = SetTimer(NULL,IDT_GAMMA_CHECK,1000*60*5,(TIMERPROC) guigamma_check);
	}else{
		if(timer_gamma_check){
			KillTimer(NULL,timer_gamma_check);
			timer_gamma_check = (UINT)NULL;
		}
	}
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
void guigamma_check(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime){

	if( timers_disabled )
		return;

	target_temp = gamma_calc_curr_target_temp(
			opt_get_lat(),opt_get_lon(),
			opt_get_temp_day(),opt_get_temp_night());
	LOG(LOGINFO,_("Gamma check, current: %d, target: %d"),
			curr_temp,target_temp);
	if( fabs((double)(curr_temp - target_temp)) >= 100 ){
		// Transition
		_gamma_toggle_timer_check(0);
		_gamma_toggle_timer_trans(1);
	}
	guimain_update_info();
	return;
}

// Disables gamma timers and checking
void guigamma_disable(void){
	(void)guigamma_set_temp(DEFAULT_DAY_TEMP);
	guimain_update_info();
	_gamma_toggle_timer_check(0);
	_gamma_toggle_timer_trans(0);
	timers_disabled = 1;
}

// Enables gamma timers
void guigamma_enable(void){
	_gamma_toggle_timer_check(1);
	timers_disabled = 0;
}

// Initialize timer to run gamma correction
void guigamma_init_timers(void){

	// Make sure gamma is synced up
	curr_temp = gamma_state_get_temperature();
	(void)gamma_state_set_temperature(curr_temp,opt_get_gamma());
	_gamma_toggle_timer_check(1);
	(void)guigamma_check((HWND)NULL,(UINT)NULL,(UINT)NULL,(DWORD)NULL);
}

// Destroys timers
void guigamma_end_timers(void){
	if( timer_gamma_check ){
		KillTimer(NULL,timer_gamma_check);
		timer_gamma_check = (UINT)NULL;
	}

	if( timer_gamma_transition ){
		KillTimer(NULL,timer_gamma_transition);
		timer_gamma_transition = (UINT)NULL;
	}
}
