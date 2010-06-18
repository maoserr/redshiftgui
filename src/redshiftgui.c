/**\file		redshiftgui.c
 * \author		Mao Yu (http://www.mao-yu.com)
 * \date		Friday, June 11, 2010
 * \brief		Main code
 * \details
 * This code is forked from the redshift project
 * (https://bugs.launchpad.net/redshift) by:
 * Jon Lund Steffensen.
 *
 * The license for this project as a whole is same (GPL v3),
 * although some components of this code (such as argument parsing)
 * were originally under different license.
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "common.h"
#include "argparser.h"
#include "options.h"
#include "gamma.h"
#ifdef ENABLE_RANDR
# include "backends/randr.h"
#endif
#ifdef ENABLE_VIDMODE
# include "backends/vidmode.h"
#endif
#ifdef ENABLE_WINGDI
# include "backends/w32gdi.h"
#endif

// Internal function to parse arguments
int _parse_options(int argc, char *argv[]){
	args_addarg("c","crt",
		_("<CRTC> CRTC to apply adjustment to (RANDR only)"),ARGVAL_STRING);
	args_addarg("g","gamma",
		_("<R:G:B> Additional gamma correction to apply"),ARGVAL_STRING);
	args_addarg("l","latlon",
		_("<LAT:LON> Latitude and longitude"),ARGVAL_STRING);
	args_addarg("m","method",
		_("<METHOD> Method to use (RANDR, VidMode, or WinGDI"),ARGVAL_STRING);
	args_addarg("n","no-gui",
		_("Run in console mode (no GUI)."),ARGVAL_NONE);
	args_addarg("o","oneshot",
		_("Adjust color and then exit (no GUI)"),ARGVAL_NONE);
	args_addarg("r","speed",
		_("<SPEED> Transition speed (default 100 K/s)"),ARGVAL_STRING);
	args_addarg("s","screen",
		_("<SCREEN> Screen to apply to"),ARGVAL_STRING);
	args_addarg("t","temps",
		_("<DAY:NIGHT> Color temperature to set at daytime/night"),ARGVAL_STRING);
	args_addarg("v","verbose",
		_("<LEVEL> Verbosity of output (0 for regular, 1 for more)"),ARGVAL_STRING);
	args_addarg("h","help",
		_("Display this help message"),ARGVAL_NONE);
	if( (args_parse(argc,argv) != ARGRET_OK) ){
		LOG(LOGERR,_("Error occurred parsing options,\n"
					"Check your config file or command line.\n"));
		args_free();
		return 0;
	}
	else{
		char *val;
		int err=0;
		char Config_file[LONGEST_PATH];
		
		if( opt_get_config_file(Config_file,LONGEST_PATH)
				&& ((args_parsefile(Config_file)) != ARGRET_OK) )
			LOG(LOGWARN,_("Invalid/empty config: %s\n"),Config_file);

		if( args_check("h") ){
			printf(_("Redshift GUI help:\n"));
			args_print();
			args_free();
			return 0;
		}

		opt_set_defaults();
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
		if( (val=args_getnamed("v")) )
			err = (!opt_set_verbose(atoi(val))) || err;
		if( err ){
			args_free();
			return 0;
		}
		if( args_unknown() ){
			printf(_("Unknown arguments encountered.\n"));
			args_free();
			return 0;
		}
	}
	args_free();
	return 1;
}

int main(int argc, char *argv[]){
	gamma_method_t method;
	if( !(_parse_options(argc,argv)) )
		return -1;
	method = gamma_init_method(opt_get_screen(),opt_get_crtc(),opt_get_method());
	if( method<0 )
		return -1;
	opt_set_method(method);
	
	if(opt_get_oneshot()){
		LOG(LOGINFO,_("Doing one-shot adjustment.\n"));
		return do_oneshot(opt_get_method(),opt_get_lat(),opt_get_lon(),
				opt_get_gamma(),opt_get_temp_day(),opt_get_temp_night());
	}

	if(opt_get_nogui()){
		LOG(LOGINFO,_("Starting in console mode.\n"));
		return do_continous(opt_get_method(),opt_get_lat(),opt_get_lon(),
				opt_get_gamma(),opt_get_temp_day(),opt_get_temp_night(),
				opt_get_trans_speed());
	}

	// Else we go to GUI mode
	return 0;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		LPSTR lpCmdLine, int nCmdShow){
	extern char ** __argv;
	extern int __argc;

	// This attaches a console to the parent process if it has a console
	if(AttachConsole(ATTACH_PARENT_PROCESS)){
		freopen("CONOUT$","wb",stdout);  // reopen stout handle as console window output
		freopen("CONOUT$","wb",stderr); // reopen stderr handle as console window output
	}
	return main(__argc,__argv);
}
#endif //_WIN32
