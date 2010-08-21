/**\file		logger.c
 * \author		Mao Yu (http://www.mao-yu.com)
 * \brief		Simple logging interface
 * \details		Logging to files, console, or callback.
 *	See logger.h for instructions. See bottom for test case.
 *
 *	This is public domain code.
 */

//#define LOG_TEST
//#define LOG_DEBUG
#define _(X) X

/*@ignore@*/
#ifdef LOG_DEBUG
# define DEBUG(...) printf(__VA_ARGS__)
#else
# define DEBUG(...)
#endif
/*@end@*/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define snprintf _snprintf
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "logger.h"

#define BUFSIZE 2048

typedef struct _loginst {
	FILE *log_fp;
	log_callback cb;
	int level;
	int levelfile;
	int levelconsole;
	int levelcb;
} LogInst;

static LogInst *global_inst = NULL;		// Global logging instance

// Initialize a logging instance to default values
static LogInst *_log_init_inst(void){
	LogInst *inst = (LogInst*)malloc(sizeof(LogInst));
	if( !inst ){
		DEBUG(_("Memory allocation error.\n"));
		return NULL;
	}
	inst->log_fp = NULL;
	inst->cb = NULL;
	inst->level = 10;
	inst->levelfile = 100;
	inst->levelconsole = 100;
	inst->levelcb = 100;
	return inst;
}

// Initialize logging interface
LogReturn log_init(LogStr filename,LogBool append,log_callback cb){
	if( !(global_inst=_log_init_inst()) )
		return LOGRET_LACK_MEM;
	if( filename ){
		LogStr fmode;
		if( append ){
			fmode = "a";
			DEBUG(_("Initializing in append mode.\n"));
		}else{
			fmode = "w";
			DEBUG(_("Initializing in write mode.\n"));
		}
		DEBUG(_("Opening the log file...\n"));
		if(!(global_inst->log_fp = fopen(filename,fmode))){
			DEBUG(_("Invalid log file name.\n"));
			return LOGRET_INVALID_LOG_FILE;
		}
	}
	DEBUG(_("Initialization ok.\n"));
	global_inst->cb = cb;
	return LOGRET_OK;
}

// Sets global logging level
LogReturn log_setlevel(int level){
	global_inst->level = level;
	return LOGRET_OK;
}

// Sets logging level for file
LogReturn log_setlevel_file(int level){
	global_inst->levelfile = level;
	return LOGRET_OK;
}

// Sets  logging level for console
LogReturn log_setlevel_console(int level){
	global_inst->levelconsole = level;
	return LOGRET_OK;
}

// Sets logging level for callback
LogReturn log_setlevel_cb(int level){
	global_inst->levelcb = level;
	return LOGRET_OK;
}

// Logs a message
LogReturn log_log(int level,
		const char *filename,
		const char *funcname,
		int linenum,
		LogStr format,...){

	if( level > global_inst->level )
		return LOGRET_OK;
	else{
		va_list args;
		static LogChar buffer[BUFSIZE];
		int charwritten=0;
		int temp;
		int bufsizeleft=BUFSIZE-1;

		DEBUG(_("Formatting log string...\n"));
		if(filename){
			temp=snprintf(buffer+charwritten,bufsizeleft,_("%s: "),filename);
			charwritten+=temp;
			bufsizeleft-=temp;
			DEBUG(_("File name: %d chars and %d chars remaining.\n"),
				charwritten,bufsizeleft);
			if( (temp<0) || (bufsizeleft<=0) )
				return LOGRET_SNPRINTF_ERR;
		}
		if(linenum){
			temp=snprintf(buffer+charwritten,bufsizeleft,_("(%d) "),linenum);
			charwritten+=temp;
			bufsizeleft-=temp;
			DEBUG(_("Line number: %d chars and %d chars remaining.\n"),
				charwritten,bufsizeleft);
			if( (temp<0) || (bufsizeleft<=0) )
				return LOGRET_SNPRINTF_ERR;
		}
		if(funcname){
			temp=snprintf(buffer+charwritten,bufsizeleft,_("[%s] "),funcname);
			charwritten+=temp;
			bufsizeleft-=temp;
			DEBUG(_("Function name: %d chars and %d chars remaining.\n"),
				charwritten,bufsizeleft);
			if( (temp<0) || (bufsizeleft<=0) )
				return LOGRET_SNPRINTF_ERR;
		}


		va_start( args, format);
		temp=vsnprintf(buffer+charwritten,bufsizeleft,format,args);
		DEBUG(_("Message: Wrote %d chars.\n"),temp);
		va_end( args );

		charwritten+=temp;
		bufsizeleft-=temp;

		if( (temp < 0) || (bufsizeleft<=0) ){
			return LOGRET_SNPRINTF_ERR;
		}else if( charwritten<(BUFSIZE-2) ){
			buffer[charwritten++]='\n';
			buffer[charwritten]='\0';
		}else{
			buffer[BUFSIZE-2]='\n';
			buffer[BUFSIZE-1]='\0';
		}

		if(global_inst->log_fp && (level<=global_inst->levelfile) ){
			DEBUG(_("Logging to file...\n"));
			if( fputs( buffer,global_inst->log_fp ) == EOF )
				return LOGRET_FILE_LOG_ERR;
		}
		if( level<=global_inst->levelconsole ){
			time_t rawtime;
			struct tm *timeinfo;
			char timebuffer[80];

			time( &rawtime );
			timeinfo = localtime( &rawtime );
			strftime(timebuffer,80,"%I:%M %p",timeinfo);
			printf("%s:",timebuffer);
			if( fputs(buffer, stdout) == EOF )
				return LOGRET_CONSOLE_LOG_ERR;
		}
		if(global_inst->cb && (level<=global_inst->levelcb) ){
			DEBUG(_("Logging to callback...\n"));
			global_inst->cb(buffer);
		}
		return LOGRET_OK;
	}
}

// Closes logging facilities
void log_end(void){
	if(global_inst->log_fp)
		fclose(global_inst->log_fp);
	free(global_inst);
	global_inst=NULL;
}

#ifdef LOG_TEST
#  define LOG(LVL,...) log_log(LVL,__FILE__,__FUNCTION__ ,__LINE__,__VA_ARGS__)
void test_cb(LogStr msg){
	printf("-+-+-+");
	fputs(msg,stdout);
}

int main(int argc, char *argv[]){
	DEBUG(_("Running test program...\n"));
	if( log_init("test.log",LOGBOOL_FALSE,test_cb) != LOGRET_OK )
		return -1;

	LOG(1,"---Testing logging functionality!");
	LOG(11,"---Can't see this! (log level is higher than default)");

	log_setlevel(11);
	LOG(11,"---Now can see this!");

	log_setlevel_console(10);
	LOG(11,"---Now only shown in file and cb!");

	log_end();
}
#endif//LOG_TEST
