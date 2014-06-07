#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "common.h"
#include "gamma.h"
#include "options.h"
#include "solar.h"
#include "location.h"
#include "systemtime.h"
#include "netutils.h"
#include "thirdparty/argparser.h"

#ifdef HAVE_SYS_SIGNAL_H
# include <sys/signal.h>
#endif

#if defined(ENABLE_IUP)
# include "gui/iupgui.h"
#elif defined(ENABLE_GTK)
# include "gui/gtkgui.h"
#elif defined(ENABLE_WINGUI)
# include "gui/win32gui.h"
#endif

// Main program return codes
#define RET_MAIN_OK 0
#define RET_MAIN_ERR -1

#ifdef ENABLE_RANDR
# define RANDR_TXT ", RANDR"
#else
# define RANDR_TXT ""
#endif
#ifdef ENABLE_VIDMODE
# define VIDMODE_TXT ", VidMode"
#else
# define VIDMODE_TXT ""
#endif
#ifdef ENABLE_WINGDI
# define WINGDI_TXT ", WinGDI"
#else
# define WINGDI_TXT ""
#endif

// Internal function to parse arguments
static int _parse_options(int argc, char *argv[]){
	(void)args_addarg("b","bright",
		_("<BRIGHTNESS> Brightness (0.1 - 1)"),ARGVAL_STRING);
	(void)args_addarg("c","crt",
		_("<CRTC> CRTC to apply adjustment to (RANDR only)"),ARGVAL_STRING);
	(void)args_addarg("g","gamma",
		_("<R:G:B> Additional gamma correction to apply"),ARGVAL_STRING);
	(void)args_addarg("l","latlon",
		_("<LAT:LON> Latitude and longitude"),ARGVAL_STRING);
	(void)args_addarg("m","method",
		_("<METHOD> Method to use (Auto" RANDR_TXT VIDMODE_TXT WINGDI_TXT ")"),ARGVAL_STRING);
	(void)args_addarg("n","no-gui",
		_("Run in console mode (no GUI)."),ARGVAL_NONE);
	(void)args_addarg("o","oneshot",
		_("Adjust color and then exit (no GUI)"),ARGVAL_NONE);
	(void)args_addarg("p","portable",
		_("Save to executable folder"),ARGVAL_NONE);
	(void)args_addarg("r","speed",
		_("<SPEED> Transition speed (default 1000 K/s)"),ARGVAL_STRING);
	(void)args_addarg("s","screen",
		_("<SCREEN> Screen to apply to"),ARGVAL_STRING);
	(void)args_addarg("t","temps",
		_("<DAY:NIGHT> Color temperature to set at daytime/night"),ARGVAL_STRING);
	(void)args_addarg("v","verbose",
		_("<LEVEL> Verbosity of output (0 = err/warn, 1 = info, 2 = verbose)"),ARGVAL_STRING);
	(void)args_addarg(NULL,"map",
		_("(Advanced) Temperature map"),ARGVAL_STRING);
	(void)args_addarg(NULL,"min",
		_("Start GUI minimized"),ARGVAL_NONE);
	(void)args_addarg("d","disable",
		_("Start GUI disabled"),ARGVAL_NONE);
	(void)args_addarg("h","help",
		_("Display this help message"),ARGVAL_NONE);
	if( (args_parse(argc,argv) != ARGRET_OK) ){
		LOG(LOGERR,_("Error occurred parsing options,"
					"Check your config file or command line."));
		return RET_FUN_FAILED;
	}else{
		char *val;
		int err=0;
		char Config_file[LONGEST_PATH];
		
		if( args_check("h")==ARGBOOL_TRUE ){
			printf(_("RedshiftGUI (%s) help:\n"),PACKAGE_VER);
			args_print();
			printf(_("\nReport bugs to %s\n"),PACKAGE_BUGREPORT);
			return RET_FUN_FAILED;
		}
		opt_init(argv[0]);
		if( (val=args_getnamed("p")) )
			err = (!opt_set_portable(1) ) || err;

		if( (opt_get_config_file(Config_file,LONGEST_PATH)
					==RET_FUN_SUCCESS)
				&& ((args_parsefile(Config_file)) != ARGRET_OK) )
			LOG(LOGWARN,_("Invalid/empty config: %s"),Config_file);


		if( (val=args_getnamed("v")) )
			err = (opt_set_verbose(atoi(val))==RET_FUN_FAILED) 
				|| err;
		if( (val=args_getnamed("b")) )
			err = (!opt_set_brightness(atof(val))) || err;
		if( (val=args_getnamed("c")) )
			err = (!opt_set_crtc(atoi(val))) || err;
		if( (val=args_getnamed("g")) )
			err = (!opt_parse_gamma(val)) || err;
		if( (val=args_getnamed("l")) )
			err = (!opt_parse_location(val)) || err;
		if( (val=args_getnamed("n")) )
			err = (!opt_set_nogui(1)) || err;
		if( (val=args_getnamed("m")) )
			err = (!opt_parse_method(val)) || err;
		if( (val=args_getnamed("o")) )
			err = (!opt_set_oneshot(1) ) || err;
		if( (val=args_getnamed("r")) )
			err = (!opt_set_transpeed(atoi(val))) || err;
		if( (val=args_getnamed("s")) )
			err = (!opt_set_screen(atoi(val))) || err;
		if( (val=args_getnamed("t")) )
			err = (!opt_parse_temperatures(val)) || err;
		if( (val=args_getnamed("min")) )
			err = (!opt_set_min(1)) || err;
		if( (val=args_getnamed("d")) )
			err = (!opt_set_disabled(1)) || err;
		if( (val=args_getnamed("map")) )
			err = (!opt_parse_map(val)) || err;
		if( err ){
			return RET_FUN_FAILED;
		}
		if( args_unknown() ){
			printf(_("Unknown arguments encountered.\n"));
			printf(_("RedshiftGUI (%s) help:\n"),PACKAGE_VER);
			args_print();
			printf(_("\nReport bugs to %s\n"),PACKAGE_BUGREPORT);
			return RET_FUN_FAILED;
		}
	}
	return RET_FUN_SUCCESS;
}

/* Change gamma and exit. */
static int _do_oneshot(void){
	int temp = gamma_calc_curr_target_temp(
				opt_get_lat(),opt_get_lon(),
				opt_get_temp_day(),opt_get_temp_night());

	LOG(LOGINFO,_("Current color temperature: %dK"),gamma_state_get_temperature());
	LOG(LOGINFO,_("Target color temperature: %dK"), temp);

	/* Adjust temperature */
	if ( !gamma_state_set_temperature(temp, opt_get_gamma()) ){
		LOG(LOGERR,_("Temperature adjustment failed."));
		return RET_FUN_FAILED;
	}
	return RET_FUN_SUCCESS;
}

#ifdef _WIN32
	static int exiting=0;
	/* Signal handler for exit signals */
	static BOOL CtrlHandler( DWORD fdwCtrlType ){
		switch( fdwCtrlType ){
		case CTRL_C_EVENT:
			LOG(LOGINFO,_("Ctrl-C event."));
			exiting=1;
			return( TRUE );
		// CTRL-CLOSE: confirm that the user wants to exit.
		case CTRL_CLOSE_EVENT:
			LOG(LOGINFO,_("Ctrl-Close event."));
			exiting=1;
			return( TRUE );
		// Pass other signals to the next handler.
		case CTRL_BREAK_EVENT:
			LOG(LOGINFO,_("Ctrl-Break event."));
			return FALSE;
		case CTRL_LOGOFF_EVENT:
			LOG(LOGINFO,_("Ctrl-Logoff event."));
			return FALSE;
		case CTRL_SHUTDOWN_EVENT:
			LOG(LOGINFO,_("Ctrl-Shutdown event."));
			return FALSE;
		default:
			return FALSE;
		}
	}
	/* Register signal handler */
	static void sig_register(void){
		if( !SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, TRUE ) )
			LOG(LOGERR,_("Unable to register Control Handler."));
	}
#elif defined(HAVE_SYS_SIGNAL_H)
	static volatile sig_atomic_t exiting = 0;
	/* Signal handler for exit signals */
	static void
	sigexit(int signo)
	{	LOG(LOGINFO,_("Detected exit signal: %d"),signo);
		exiting = 1;}
	/* Register signal handler */
	static void sig_register(void){
		struct sigaction sigact;
		sigact.sa_handler = sigexit;
		(void)sigemptyset(&sigact.sa_mask);
		sigact.sa_flags = 0;
		/*@i@*/(void)sigaction(SIGINT, &sigact, NULL);
		/*@i@*/(void)sigaction(SIGTERM, &sigact, NULL);
	/*@i@*/}
#else /* ! HAVE_SYS_SIGNAL_H */
	static int exiting = 0;
#	define sig_register()
#endif /* ! HAVE_SYS_SIGNAL_H */

static void transition_to_temp(int curr, int target, int speed){
	int currtemp = curr;

	do{
		if( curr > target ){
			currtemp-=speed/10;
			if( currtemp < target )
				break;
		}else{
			currtemp+=speed/10;
			if( currtemp > target )
				break;
		}

		LOG(LOGVERBOSE,_("Transition color: %dK"),currtemp);
		if( !gamma_state_set_temperature(currtemp,opt_get_gamma()) ){
			LOG(LOGERR,_("Temperature adjustment failed."));
			exiting = 1;
			break;
		}
		/*@i@*/SLEEP(100);
	}while(!exiting);

	LOG(LOGVERBOSE,_("Target color reached: %dK"),target);
	if( !gamma_state_set_temperature(target,opt_get_gamma()) ){
		LOG(LOGERR,_("Temperature adjustment failed."));
		exiting = 1;
	}
}

/* Change gamma continuously until break signal. */
static int _do_console(void)
{
	int target_temp;
	int transpeed = opt_get_trans_speed();
	int sec_countdown=0;
	int saved_temp = gamma_state_get_temperature();
	int curr_temp = saved_temp;

	LOG(LOGVERBOSE,_("Original temp: %dK"),saved_temp);
	sig_register();
	do{
		// Re-check every 20 minutes
		if( sec_countdown <= 0 ){
			curr_temp=gamma_state_get_temperature();
			target_temp=gamma_calc_curr_target_temp(
				opt_get_lat(),opt_get_lon(),
				opt_get_temp_day(),opt_get_temp_night());
			transition_to_temp(curr_temp,target_temp,transpeed);
			sec_countdown = 60*20;
		}else{
			--sec_countdown;
		}
		LOG(LOGVERBOSE,_("Countdown: %d"),sec_countdown);
		SLEEP(1000);
	}while(!exiting);
	exiting=0;
	curr_temp=gamma_state_get_temperature();
	// Use a constant 2000K/s transition speed to exit
	transition_to_temp(curr_temp,DEFAULT_DAY_TEMP,2000);
	return RET_FUN_SUCCESS;
}

int main(int argc, char *argv[]){
	gamma_method_t method;
	int ret=RET_MAIN_ERR;

#ifdef _WIN32
	// This attaches a console to the parent process if it has a console
	if(AttachConsole(ATTACH_PARENT_PROCESS)){
		// reopen stout handle as console window output
		(void)freopen("CONOUT$","wb",stdout);
		// reopen stderr handle as console window output
		(void)freopen("CONOUT$","wb",stderr);
	}
#endif

	if( log_init(NULL,LOGBOOL_FALSE,NULL) != LOGRET_OK ){
		printf(_("Could not initialize logger.\n"));
		return RET_MAIN_ERR;
	}

	if( !(_parse_options(argc,argv)) )
		goto end;

	// Initialize gamma method
	if( !gamma_load_methods() )
		goto end;

	method = gamma_init_method(opt_get_screen(),opt_get_crtc(),
			opt_get_method());
	if( !method )
		goto end;

	// Initialize location method
	if( !net_init() ){
		(void)gamma_state_free();
		goto end;
	}
	
	if(opt_get_oneshot()){
		// One shot mode
		LOG(LOGVERBOSE,_("Doing one-shot adjustment."));
		ret = _do_oneshot();
	}else if(opt_get_nogui()){
		// Console mode
		LOG(LOGVERBOSE,_("Starting in console mode."));
		ret = _do_console();
	}else{
		// GUI mode
		LOG(LOGINFO,_("Starting in GUI mode."));
#if defined(ENABLE_IUP)
	ret = iup_gui(argc,argv);
#elif defined(ENABLE_GTK)
	ret = gtk_gui(argc,argv);
#elif defined(ENABLE_WINGUI)
	ret = win32_gui(argc,argv);
#else
		LOG(LOGVERBOSE,_("No GUI toolkit compiled in."));
		ret = RET_FUN_FAILED;
#endif
	}
	(void)net_end();
	(void)gamma_state_free();

	end:
	opt_free();
	args_free();
	log_end();

	if( ret )
		return RET_MAIN_OK;
	else
		return RET_MAIN_ERR;
}

#if defined(_WIN32) && !defined(ENABLE_IUP)
// Win32 wrapper function for GUI mode
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		LPSTR lpCmdLine, int nCmdShow){
	extern char ** __argv;
	extern int __argc;

	return main(__argc,__argv);
}
#endif //_WIN32

