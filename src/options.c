#include "common.h"
#include "gamma.h"
#include "options.h"
#include "solar.h"
#include "gamma_vals.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>
#define SIZEOF(X) (sizeof(X)/sizeof(X[0]))

/**\brief Redshift options.*/
typedef struct{
	/**\brief Brightness */
	float brightness;
	/**\brief Latitude */
	float lat;
	/**\brief Longitude */
	float lon;
	/**\brief Daytime temperature */
	int temp_day;
	/**\brief Nighttime temperature */
	int temp_night;
	/**\brief Gamma changing method */
	gamma_method_t method;
	/**\brief Screen number */
	int screen_num;
	/**\brief Additional gamma adjustment */
	gamma_s gamma;
	/**\brief CRTC number (RANDR only) */
	int crtc_num;
	/**\brief Transition speed */
	int trans_speed;
	/**\brief Oneshot mode enabled? */
	int one_shot;
	/**\brief Portable mode */
	int portable;
	/**\brief Console mode enabled? */
	int nogui;
	/**\brief Verbosity level */
	int verbose;
	/**\brief Start GUI minimized */
	int startmin;
	/**\brief Start GUI disabled */
	int startdisabled;
	/**\brief Temperature map (Advanced) */
	/*@null@*//*@partial@*//*@owned@*/ pair *map;
	/**\brief Temperature map size (Advanced) */
	int map_size;
	/**\brief Folder for options */
	char exepath[LONGEST_PATH];
} rs_opts;

static rs_opts Rs_opts;
static pair default_map[]={
	{177.0,	100},
	{3.0,	100},
	{-6.0,	0},
	{-174.0,0},
};

// Checks to see if config file exists in directory
static int _config_exist(char *basedir){

	return RET_FUN_FAILED;
}

/* Retrieves configuration file full path */
int opt_get_config_file(char buffer[],size_t bufsize){
		char *configdir;
		size_t s_home;

		if( opt_get_portable() ){
			configdir = Rs_opts.exepath ;
		}else{
#ifndef _WIN32
			configdir = getenv("HOME");
#else
			configdir = getenv("APPDATA");
#endif
		}
		if( configdir==NULL ){
			strcpy(buffer,"None");
			return RET_FUN_FAILED;
		}
		s_home = strlen(configdir);
		if( s_home < (bufsize+strlen(RSG_RCFILE)-5) ){
			strcpy(buffer,configdir);
			buffer[s_home++]=PATH_SEP;
			strcpy(buffer+s_home,RSG_RCFILE);
			return RET_FUN_SUCCESS;
		}
		strcpy(buffer,"TooLong");
		return RET_FUN_FAILED;
}

// Load defaults
void opt_init(char *exename){
	struct stat buffer;   
	char *sep = strrchr(exename,PATH_SEP);
	char pathbuffer[LONGEST_PATH];
	Rs_opts.map=NULL;
	(void)opt_set_verbose(0);
	(void)opt_set_brightness(1.0);
	(void)opt_set_location(0,0);
	(void)opt_set_temperatures(DEFAULT_DAY_TEMP,DEFAULT_NIGHT_TEMP);
	(void)opt_set_gamma(DEFAULT_GAMMA,DEFAULT_GAMMA,DEFAULT_GAMMA);
	(void)opt_set_method(GAMMA_METHOD_AUTO);
	(void)opt_set_screen(-1);
	(void)opt_set_crtc(-1);
	(void)opt_set_transpeed(1000);
	(void)opt_set_oneshot(0);
	(void)opt_set_nogui(0);
	if(sep && (sep<(exename+LONGEST_PATH-10))){
		strncpy(Rs_opts.exepath,exename,sep-exename+1);
	}else{
		Rs_opts.exepath[0]='.';
		Rs_opts.exepath[1]=PATH_SEP;
		Rs_opts.exepath[2]='\0';
	}
	strcpy(pathbuffer,Rs_opts.exepath);
	strcpy(pathbuffer+strlen(pathbuffer),RSG_RCFILE);
	if(stat (pathbuffer, &buffer) == 0){
		opt_set_portable(1);
	}
	(void)opt_set_min(0);
	(void)opt_set_disabled(0);
}

// Sets brightness
int opt_set_brightness(double brightness){
	Rs_opts.brightness = (float)brightness;
	LOG(LOGVERBOSE,_("Option brightness: %f"),Rs_opts.brightness);
	return RET_FUN_SUCCESS;
}

// Sets the CRTC
int opt_set_crtc(int val){
	Rs_opts.crtc_num = val;
	return RET_FUN_SUCCESS;
}

// Sets the gamma
int opt_set_gamma(float r, float g, float b){
	Rs_opts.gamma.r = r;
	Rs_opts.gamma.g = g;
	Rs_opts.gamma.b = b;
	return RET_FUN_SUCCESS;
}

// Parses a gamma argument by the form of "R:G:B"
int opt_parse_gamma(char *val){
	char *s;
	s = strchr(val, ':');
	if (s == NULL) {
		/* Use value for all channels */
		float g = (float) atof(val);
		return opt_set_gamma(g,g,g);
	} else {
		char *gs;
		float r,g,b;
		/* Parse separate value for each channel */
		*(s++) = '\0';
		r = (float) atof(val); /* Red */
		gs = s;
		s = strchr(s, ':');
		if (s == NULL) {
			LOG(LOGERR,_("Malformed gamma argument: %s.\n"),val);
			return RET_FUN_FAILED;
		}
		*(s++) = '\0';
		g = (float) atof(gs); /* Green */
		b = (float) atof(s); /*  Blue */
		return opt_set_gamma(r,g,b);
	}
}

// Sets the location
int opt_set_location(float lat, float lon){
	Rs_opts.lat = lat;
	Rs_opts.lon = lon;
	LOG(LOGVERBOSE,_("Latitude/longitude set to: %.2f,%.2f"),lat,lon);
	return RET_FUN_SUCCESS;
}

// Parses the location argument in the form of "LAT:LON"
int opt_parse_location(char *val){
	char *s;
	float lat,lon;
	s = strchr(val, ':');
	if (s == NULL) {
		LOG(LOGERR,_("Malformed location argument: %s.\n"),val);
		return RET_FUN_FAILED;
	}
	*(s++) = '\0';
	lat = (float) atof(val);
	lon = (float) atof(s);
	return opt_set_location(lat,lon);
}

// Set no GUI mode
int opt_set_nogui(int val){
	Rs_opts.nogui = val;
	return RET_FUN_SUCCESS;
}

// Sets the method to change the monitor temperature
int opt_set_method(gamma_method_t method){
	Rs_opts.method = method;
	return RET_FUN_SUCCESS;
}

// Parses the method to change the monitor temperature
int opt_parse_method(char *val){
	int ret;
	if (strcmp(val, "auto") == 0 || strcmp(val, "Auto") == 0 ){
		ret = opt_set_method(GAMMA_METHOD_AUTO);
	}else if (strcmp(val, "randr") == 0 || strcmp(val, "RANDR") == 0) {
#ifdef ENABLE_RANDR
		ret = opt_set_method(GAMMA_METHOD_RANDR);
#else
		LOG(LOGERR,_("RANDR method was not enabled at compile time.\n"));
		ret = RET_FUN_FAILED;
#endif
	} else if (strcmp(val, "vidmode") == 0 ||strcmp(val, "VidMode") == 0) {
#ifdef ENABLE_VIDMODE
	    ret = opt_set_method(GAMMA_METHOD_VIDMODE);
#else
		LOG(LOGERR,_("VidMode method was not enabled at compile time.\n"));
		ret = RET_FUN_FAILED;
#endif
	} else if (strcmp(val, "wingdi") == 0 || strcmp(val, "WinGDI") == 0) {
#ifdef ENABLE_WINGDI
		ret = opt_set_method(GAMMA_METHOD_WINGDI);
#else
		LOG(LOGERR,_("WinGDI method was not enabled at compile time.\n"));
		ret = RET_FUN_FAILED;
#endif
	} else {
		LOG(LOGERR,_("Unknown method `%s'.\n"),val);
		ret = RET_FUN_FAILED;
	}
	return ret;

}

// Sets oneshot mode
int opt_set_oneshot(int onoff){
	Rs_opts.one_shot = onoff;
	return RET_FUN_SUCCESS;
}

// Set portable mode
int opt_set_portable(int onoff){
	Rs_opts.portable = onoff;
	return RET_FUN_SUCCESS;
}

// Sets transition - change in temperature per second
int opt_set_transpeed(int tpersec){
	Rs_opts.trans_speed = tpersec;
	return RET_FUN_SUCCESS;
}

// Sets the screen to apply adjustment to
int opt_set_screen(int val){
	Rs_opts.screen_num = val;
	return RET_FUN_SUCCESS;
}

// Sets the temperature for day/night
int opt_set_temperatures(int tday, int tnight){
	Rs_opts.temp_day = tday;
	Rs_opts.temp_night = tnight;
	return RET_FUN_SUCCESS;
}

// Parses the transition temperature string in the form of "DAY:NIGHT"
int opt_parse_temperatures(char *val){
	char *s = strchr(val, ':');
	int temp_day, temp_night;
	if (s == NULL) {
		LOG(LOGERR,_("Malformed temperature argument: %s.\n"),val);
		return RET_FUN_FAILED;
	}
	*(s++) = '\0';
	temp_day = atoi(val);
	temp_night = atoi(s);
	return opt_set_temperatures(temp_day, temp_night);
}

// Sets the verbosity of logging
int opt_set_verbose(int level){
	Rs_opts.verbose = level;
	(void)log_setlevel(level+2);
	return RET_FUN_SUCCESS;
}

// Sets start minimized
int opt_set_min(int val){
	Rs_opts.startmin = val;
	return RET_FUN_SUCCESS;
}

// Sets start disabled
int opt_set_disabled(int val){
	Rs_opts.startdisabled = val;
	return RET_FUN_SUCCESS;
}

// Parse temperature map
int opt_parse_map(char *map){
	char *currstr=map; /* Pointer string */
	char *currsep,*currend;
	int cnt=0;
	int i;
	double prevelev=SOLAR_MAX_ANGLE;
	pair *curr_map;
	while( (currstr=strchr(currstr,',')) ){
		if( (currstr=strchr(currstr,';')) )
			++cnt;
		else
			break;
	}
	if( cnt<=0 ){
		LOG(LOGERR,_("Map empty."));
		return RET_FUN_FAILED;
	}
	curr_map = (pair*)malloc(sizeof(pair)*cnt);
	if( !curr_map ){
		LOG(LOGERR,_("Map memory allocation error"));
		return RET_FUN_FAILED;
	}
	currstr=map;
	setlocale(LC_NUMERIC,"");
	for( i=0; i<cnt; ++i ){
		currend=strchr(currstr,';');
		curr_map[i].elev=strtod(currstr,&currsep);
		curr_map[i].temp=atof(++currsep);
		if( curr_map[i].elev > prevelev ){
			free(curr_map);
			LOG(LOGERR,_("Invalid map line, elevation must be decreasing."));
			return RET_FUN_FAILED;
		}
		prevelev = curr_map[i].elev;
		if( (curr_map[i].temp>100.0)
				/*@i@*/|| (curr_map[i].temp<0.0) ){
			free(curr_map);
			LOG(LOGERR,_("Invalid map line, temperature must be between 0-100%%."));
			return RET_FUN_FAILED;
		}
		
		/*@i@*/LOG(LOGVERBOSE,_("Map line: %f,%f"),curr_map[i].elev,
				curr_map[i].temp);
		currstr=++currend;
	}
	if( Rs_opts.map )
		free(Rs_opts.map);
	Rs_opts.map = curr_map;
	Rs_opts.map_size=cnt;
	return RET_FUN_SUCCESS;
}

float opt_get_brightness(void)
{return Rs_opts.brightness;}

int opt_get_crtc(void)
{return Rs_opts.crtc_num;}

gamma_s opt_get_gamma(void)
{return Rs_opts.gamma;}

float opt_get_lat(void)
{return Rs_opts.lat;}

float opt_get_lon(void)
{return Rs_opts.lon;}

int opt_get_nogui(void)
{return Rs_opts.nogui;}

gamma_method_t opt_get_method(void)
{return Rs_opts.method;}

int opt_get_oneshot(void)
{return Rs_opts.one_shot;}

int opt_get_portable(void)
{return Rs_opts.portable;}

int opt_get_trans_speed(void)
{return Rs_opts.trans_speed;}

int opt_get_screen(void)
{return Rs_opts.screen_num;}

int opt_get_temp_day(void)
{return Rs_opts.temp_day;}

int opt_get_temp_night(void)
{return Rs_opts.temp_night;}

int opt_get_verbosity(void)
{return Rs_opts.verbose;}

int opt_get_min(void)
{return Rs_opts.startmin;}

int opt_get_disabled(void)
{return Rs_opts.startdisabled;}

pair *opt_get_map(int *size){
	if( !Rs_opts.map ){
		(*size)=(int)SIZEOF(default_map);
		return default_map;
	}else{
		(*size)=(int)Rs_opts.map_size;
		return Rs_opts.map;
	}
}

temp_gamma *opt_get_gammap(int *size){
	(*size)=(int)SIZEOF(blackbody_color);
	return blackbody_color;
}

/* Writes the configuration file based on current state */
void opt_write_config(void){
	char Config_file[LONGEST_PATH];
	FILE *fid_config;

	if(opt_get_config_file(Config_file,LONGEST_PATH)
			!=RET_FUN_SUCCESS)
		return;
	fid_config = fopen(Config_file,"w");
	if( fid_config==NULL )
		return;
	if( opt_get_min()!=0 )
		fprintf(fid_config,"min\n");
	if( opt_get_disabled()!=0 )
		fprintf(fid_config,"disable\n");
	fprintf(fid_config,"temps=%d:%d\n",opt_get_temp_day(),opt_get_temp_night());
	fprintf(fid_config,"latlon=%f:%f\n",opt_get_lat(),opt_get_lon());
	fprintf(fid_config,"speed=%d\n",opt_get_trans_speed());
	fprintf(fid_config,"method=%s\n",gamma_get_method_name(opt_get_method()));
	if( Rs_opts.map ){
		int i;
		fprintf(fid_config,"map=");
		for( i=0; i<Rs_opts.map_size; ++i )
			fprintf(fid_config,"%.2f|%.2f;",Rs_opts.map[i].elev,Rs_opts.map[i].temp);
		fprintf(fid_config,"\n");
	}
	(void)fclose(fid_config);
}

/* Frees resources used by options */
void opt_free(void){
	LOG(LOGVERBOSE,_("Freeing options"));
	if( Rs_opts.map )
		free(Rs_opts.map);
}
