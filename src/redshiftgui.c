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

#ifndef _WIN32
#define CONFIG_FILE "$HOME/.redshiftgrc"
#else
#define CONFIG_FILE "%APPDATA%/redshiftg.ini"
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
	args_addarg("o","oneshot",
		_("Adjust color and then exit"),ARGVAL_NONE);
	args_addarg("r","speed",
		_("<SPEED> Transition speed (default 100 K/s)"),ARGVAL_STRING);
	args_addarg("s","screen",
		_("<SCREEN> Screen to apply to"),ARGVAL_STRING);
	args_addarg("t","temperatures",
		_("<DAY:NIGHT> Color temperature to set at daytime/night"),ARGVAL_STRING);
	args_addarg("v","verbose",
		_("<LEVEL> Verbosity of output (0 for regular, 1 for more)"),ARGVAL_STRING);
	args_addarg("h","help",
		_("Display this help message"),ARGVAL_NONE);
	if( (args_parse(argc,argv) != ARGRET_OK) ){
		LOG(LOGERR,_("Error occurred parsing options,\n"
					"Check your config file or command line.\n"));
		return 0;
	}
	else{
		char *val;
		int err=0;

		if( (args_parsefile(CONFIG_FILE)) != ARGRET_OK )
			LOG(LOGWARN,_("No config file or invalid config file format, skipping...\n"));

		if( args_check("h") ){
			printf(_("Redshift GUI help:\n"));
			args_print();
			return 0;
		}

		opt_set_defaults();
		if( (val=args_getnamed("c")) )
			err = (!opt_set_crtc(atoi(val))) || err;
		if( (val=args_getnamed("g")) )
			err = (!opt_parse_gamma(val)) || err;
		if( (val=args_getnamed("l")) )
			err = (!opt_parse_location(val)) || err;
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
		if( err )
			return 0;
		if( args_unknown() )
			return 0;
	}
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
	
	if(opt_get_oneshot())
		return do_oneshot(opt_get_method(),opt_get_lat(),opt_get_lon(),
				opt_get_gamma(),opt_get_temp_day(),opt_get_temp_night(),
				opt_get_verbosity());

	return 0;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		LPSTR lpCmdLine, int nCmdShow){
	extern char ** __argv;
	extern int __argc;
	int r;

	// This attaches a console to the parent process if it has a console
	if(AttachConsole(ATTACH_PARENT_PROCESS)){
		freopen("CONOUT$","wb",stdout);  // reopen stout handle as console window output
		freopen("CONOUT$","wb",stderr); // reopen stderr handle as console window output
	}
	printf("\n\n");
	r = main(__argc,__argv);
	printf("\nPress enter...\n");
	return r;
}
#endif //_WIN32
