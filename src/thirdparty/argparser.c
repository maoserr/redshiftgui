/**\file		argparser.c
 * \author		Mao Yu (http://www.mao-yu.com)
 * \brief		Parses command line argument given argc and argv.
 * \details		Argument parsing functionality.
 *	See argparser.h for instructions. See bottom for test case.
 *
 *	This is public domain code.
 */

//#define ARGS_TEST
//#define ARGS_DEBUG
#define _(X) X

/*@ignore@*/
#ifdef ARGS_DEBUG
# define DEBUG(...) printf(__VA_ARGS__)
#else
# define DEBUG(...)
#endif
/*@end@*/

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "argparser.h"

typedef struct _argitem {
	ArgStr shortname;
	ArgStr longname;
	ArgStr helpstring;
	ArgValType valtype;
	ArgBool used;
	ArgStr value;
	struct _argitem *next;
} ArgItem;

static ArgItem *validitems = NULL;		// Valid arguments
static ArgItem *unknown = NULL;			// Unknown arguments
static ArgItem *unnamed = NULL;			// Additional unnamed arguments
static ArgBool parsed = ARGBOOL_FALSE;	// Whether parsed or not
static const unsigned int MAX_LONG_LEN = 10;	// Maximum length of long names
static const unsigned int MAX_HELP_LEN = 2048;	// Maximum length of help strings

// Internal function to validate inputs
static ArgReturn _args_validate_inputs(
		const ArgStr shortname,
		const ArgStr longname,
		const ArgStr helpstring)
{
	if( !(shortname || longname) )
		return ARGRET_NO_NAMES;

	if( shortname ){
		size_t slen = strlen(shortname);
		if( slen > 2 )
			return ARGRET_INVALID_SNAME;
	}
	if( longname ){
		size_t llen = strlen(longname);
		if( (llen<3) || (llen>MAX_LONG_LEN) )
			return ARGRET_INVALID_LNAME;
	}
	if( helpstring ){
		size_t hlen = strlen(helpstring);
		if( hlen > MAX_HELP_LEN )
			return ARGRET_INVALID_HELP;
	}
	return ARGRET_OK;
}

// Function to add a valid argument
ArgReturn args_addarg(
		const ArgStr shortname,
		const ArgStr longname,
		const ArgStr helpstring,
		const ArgValType valtype)
{
	ArgReturn ret = ARGRET_OK;
	ArgItem *newitem = (ArgItem*)malloc(sizeof(ArgItem));
	static ArgItem empty = {0};
	static ArgItem *lastadded=NULL;

	if( !newitem )
		return ARGRET_MEM_ERROR;

	// Validate inputs
	ret = _args_validate_inputs(shortname,longname,helpstring);
	if( ret != ARGRET_OK ){
		free(newitem);
		return ret;
	}

	// Create the structure
	*newitem = empty;
	if(shortname){
		newitem->shortname = (ArgStr)malloc(
				sizeof(ArgChar)*(strlen(shortname)+1));
		if( !(newitem->shortname) ){
			ret = ARGRET_MEM_ERROR;
			free(newitem);
			return ret;
		}
		strcpy(newitem->shortname, shortname);
	}
	if(longname){
		newitem->longname = (ArgStr)malloc(
				sizeof(ArgChar)*(strlen(longname)+1));
		if( !(newitem->longname) ){
			ret = ARGRET_MEM_ERROR;
			free(newitem->shortname);
			free(newitem);
			return ret;
		}
		strcpy(newitem->longname,longname);
	}
	if(helpstring){
		newitem->helpstring = (ArgStr)malloc(
				sizeof(ArgChar)*(strlen(helpstring)+1));
		if( !(newitem->helpstring) ){
			ret = ARGRET_MEM_ERROR;
			free(newitem->shortname);
			free(newitem->longname);
			free(newitem);
			return ret;
		}
		strcpy(newitem->helpstring,helpstring);
	}
	newitem->valtype = valtype;
	newitem->used = ARGBOOL_FALSE;
	if( lastadded )
		lastadded->next = newitem;
	else
		validitems = newitem;
	lastadded = newitem;
	return ret;
}

// Internal function to set an argument's value
static ArgItem *_args_setval(
		const ArgStr name,
		const ArgStr value)
{
	ArgItem *foundvalid = NULL;
	ArgItem *currvalid = validitems;
	ArgStr shortname=NULL;
	ArgStr longname=NULL;

	if( !name )
		return NULL;
	if( strlen(name) <= 2 )
		shortname = name;
	else
		longname = name;

	if( _args_validate_inputs(shortname,longname,NULL)
			!= ARGRET_OK )
		return NULL;

	while( currvalid ){
		if( shortname && currvalid->shortname &&
				(strcmp( currvalid->shortname, shortname )==0) ){
			foundvalid = currvalid;
			break;
		}else if( longname && currvalid->longname &&
				(strcmp( currvalid->longname,longname )==0) ){
			foundvalid = currvalid;
			break;
		}
		currvalid = currvalid->next;
	}
	if( foundvalid ){
		DEBUG("Found valid argument -%s/--%s\n",
				foundvalid->shortname,
				foundvalid->longname);

		if( (foundvalid->valtype != ARGVAL_NONE)
				&& value){
			if( !(foundvalid->value) ){
				foundvalid->value = malloc(
					sizeof(ArgChar)*(strlen(value)+1));
				strcpy(foundvalid->value, value);
				DEBUG("Value %s added to name: -%s/--%s.\n",
					value,foundvalid->shortname,
					foundvalid->longname);
			}
		}
		foundvalid->used = ARGBOOL_TRUE;
		return foundvalid;
	}
	return NULL;
}

// Internal function to store unknown arguments
static ArgItem *_args_setunknown(const ArgStr arg){
	ArgItem *newitem = (ArgItem*)malloc(sizeof(ArgItem));
	static ArgItem *lastadded = NULL;

	if( !newitem )
		return NULL;
	newitem->value = (ArgStr)malloc(sizeof(ArgChar)*(strlen(arg)+1));
	if( !(newitem->value ) ){
		free(newitem);
		return NULL;
	}
	strcpy(newitem->value, arg);
	newitem->next = NULL;
	if( lastadded )
		lastadded->next = newitem;
	else
		unknown = newitem;
	lastadded = newitem;
	return newitem;
}

// Internal function to store additional unnamed arguments
static ArgItem *_args_setunnamed(const ArgStr arg){
	ArgItem *newitem = (ArgItem*)malloc(sizeof(ArgItem));
	static ArgItem *lastadded = NULL;

	if( !newitem )
		return NULL;
	newitem->value = (ArgStr)malloc(sizeof(ArgChar)*(strlen(arg)+1));
	if( !(newitem->value ) ){
		free(newitem);
		return NULL;
	}
	strcpy(newitem->value, arg);
	newitem->next = NULL;
	if( lastadded )
		lastadded->next = newitem;
	else
		unnamed = newitem;
	lastadded = newitem;
	return newitem;
}

#ifdef ARGS_DEBUG
// Internal function to print all found arguments
void _args_DEBUGprintfound(void)
{
	const ArgItem *curr=validitems;
	while(curr){
		if( curr->used == ARGBOOL_TRUE ){
			if( curr->shortname )
				printf(_("  -%-2s"),curr->shortname);
			else
				printf(_("   %-2s"),"");
			if( curr->longname )
				printf(_(" --%-12s"),curr->longname);
			else
				printf(_("   %-12s"),"");
			printf(_(" %s\n"),curr->value);
		}
		curr = curr->next;
	}
	curr = unnamed;
	while(curr){
		printf("  Unnamed: %s\n",curr->value);
		curr = curr->next;
	}
	curr = unknown;
	while(curr){
		printf("  Unknown: %s\n",curr->value);
		curr = curr->next;
	}
	DEBUG("--End found arguments.\n\n");
}
#endif

// Function to parse the arguments
ArgReturn args_parse(int argc, char *argv[]){
	int i = 1;
	char *currarg;
	char *nextarg;
	size_t currlen;
	ArgItem *curritem;
	while( i<argc ){
		currarg = argv[i];
		if( i+1<argc )
			nextarg = argv[i+1];
		else
			nextarg = NULL;
		currlen = strlen(currarg);
		curritem = NULL;

		if( (currlen>0) && (currarg[0] == '-' ) ){
			// Check if it's a long
			if( (currlen>1) && (currarg[1] == '-') ){
				DEBUG("Found a long argument: %s\n",currarg);
				curritem = _args_setval(currarg+2,nextarg);
			}else{
				DEBUG("Found a short argument: %s\n",currarg);
				curritem = _args_setval(currarg+1,nextarg);
			}
			if( !curritem ){
				DEBUG("Unknown argument found: %s\n",
					currarg);
				_args_setunknown(currarg);
			}
		}else{
			DEBUG("Found an unnamed argument: %s\n",currarg);
			_args_setunnamed(currarg);
		}
		++i;
		if( curritem && (curritem->valtype != ARGVAL_NONE) ){
			++i;
		}
		DEBUG("\n");
	}
#ifdef ARGS_DEBUG
	DEBUG("--Printing found arguments after command line.\n");
	_args_DEBUGprintfound();
#endif
	parsed = ARGBOOL_TRUE;
	return ARGRET_OK;
}

static ArgReturn _parse_line(ArgChar buffer[]){
	ArgChar name[11];
	char *sep;
	ArgItem *item;
	
	if( buffer[0] == '\0' ){
		DEBUG("End of configuration.\n");
		return ARGRET_FILE_READ_ERROR;
	}

	sep = strchr(buffer,'=');
	DEBUG("Parsing line %s\n",buffer);
	if( sep && (sep-buffer) <= 10 ){
		(*sep) = '\0';
		strncpy(name,buffer,10);
		item = _args_setval(name,++sep);
		if( !item ){
			DEBUG("Found unknown value %s.\n",name);
			_args_setunknown(name);
		}
	}else if( strlen(buffer)<=10 ){
		DEBUG("Found flag %s.\n",buffer);
		item = _args_setval(buffer,NULL);
		if( !item ){
			DEBUG("Found unknown %s.\n",buffer);
			_args_setunknown(buffer);
		}
	}
	return ARGRET_OK;
}

// Function to parse arguments from a file
ArgReturn args_parsefile(ArgStr filename)
{
	ArgChar *buffer;
	int buflen=1024;
	int currchar;
	fpos_t currpos;
	int currlen=0;
	FILE *fp = fopen(filename,"r");
	if ( !fp )
		return ARGRET_INVALID_FILE;
	
	buffer = (ArgChar*)malloc(sizeof(ArgChar)*buflen);

	fgetpos( fp, &currpos );
	while( 1 ){
		currchar = fgetc(fp);
		DEBUG("%c",currchar);
		++currlen;
		if( (currchar==EOF) || (currchar=='\n') ){
			int i;
			if( currchar==EOF ){
				DEBUG("\nEOF Detected");
			}
			DEBUG("\nFound newline or EOF (len %d)\n",currlen);
			if( currlen >= buflen )
				buffer = (ArgChar*)realloc(buffer,currlen);

			fsetpos(fp,&currpos);
			for( i=0; i<currlen; ++i){
				buffer[i]=fgetc(fp);
			}
			buffer[--i]='\0';
			DEBUG("Buffer contents: %s\n",buffer);
			// Skip lines starting with # or empty lines
			if( (buffer[0]!='#') && (buffer[0]!='\n') )
				_parse_line(buffer);
			fgetpos(fp, &currpos);
			currlen = 0;
		}
		if( (currchar==EOF) )
			break;
	}
	free(buffer);
	if( ferror(fp) ){
		fclose(fp);
		return ARGRET_FILE_READ_ERROR;
	}
	fclose(fp);
#ifdef ARGS_DEBUG
	DEBUG("--Printing found arguments after config file.\n");
	_args_DEBUGprintfound();
#endif
	return ARGRET_OK;
}

// Internal function to search for item
static ArgItem *_args_search(const ArgStr name)
{
	ArgItem *found=NULL;
	ArgItem *curr=validitems;
	ArgStr shortname=NULL;
	ArgStr longname=NULL;

	if( !name )
		return NULL;

	if( strlen(name) <=2 )
		shortname = name;
	else
		longname = name;

	if( _args_validate_inputs(shortname,longname,NULL)
			!= ARGRET_OK)
		return NULL;

	if( shortname ){
		while(curr){
			if( (curr->used == ARGBOOL_TRUE) &&
					curr->shortname &&
					strcmp(curr->shortname,shortname) == 0 ){
				found = curr;
				break;
			}
			curr = curr->next;
		}
	}else if( longname ){
		while(curr){
			if( (curr->used == ARGBOOL_TRUE) &&
					curr->longname &&
					strcmp(curr->longname,longname) == 0 ){
				found = curr;
				break;
			}
			curr = curr->next;
		}
	}
	return found;
}

// Function to check if an argument was found
ArgBool args_check(const ArgStr name)
{
	ArgItem *found=_args_search(name);
	if( found ){
		found->used = ARGBOOL_TRUE;
		return ARGBOOL_TRUE;
	}else
		return ARGBOOL_FALSE;
}

// Function to get unnamed argument
ArgStr args_getunnamed(const int index)
{
	ArgItem *curr = unnamed;
	int i=1;
	while(curr && i<index){
		curr = curr->next;
		++i;
	}
	if( i == index && curr )
		return curr->value;
	else
		return NULL;
}

// Function to get the argument's value
ArgStr args_getnamed(const ArgStr name)
{
	ArgItem *found = _args_search(name);
	if( found ){
		if( found->value )
			return found->value;
		else
			return _("");
	}else
		return NULL;
}

// Function to print a list of the valid arguments
void args_print(void)
{
	const ArgItem *curr=validitems;
	while(curr){
		if( curr->shortname )
			printf(_("  -%-2s"),curr->shortname);
		else
			printf(_("   %-2s"),"");
		if( curr->longname )
			printf(_(" --%-12s"),curr->longname);
		else
			printf(_("   %-12s"),"");
		printf(_(" %s\n"),curr->helpstring);
		curr = curr->next;
	}
}

// Function to print a list of unknown arguments
ArgBool args_unknown(void)
{
	const ArgItem *curr=unknown;
	if( !unknown )
		return ARGBOOL_FALSE;

	while(curr){
		if( curr->value )
			printf(_("Unknown argument: %s\n"),curr->value);
		curr = curr->next;
	}
	return ARGBOOL_TRUE;
}

// Function to free all resources
void args_free(void)
{
	ArgItem *curr = unknown;
	ArgItem *next;
	DEBUG("Freeing unknown arguments.\n");
	while(curr){
		next = curr->next;
		free(curr->value);
		free(curr);
		curr = next;
	}
	DEBUG("Freeing unnamed arguments.\n");
	curr = unnamed;
	while(curr){
		next = curr->next;
		free(curr->value);
		free(curr);
		curr = next;
	}
	DEBUG("Freeing valid arguments.\n");
	curr = validitems;
	while(curr){
		next = curr->next;
		free(curr->shortname);
		free(curr->longname);
		free(curr->helpstring);
		free(curr->value);
		free(curr);
		curr = next;
	}
	validitems = NULL;
	unknown = NULL;
	unnamed = NULL;
}

#ifdef ARGS_TEST
int main( int argc, char *argv[] ){
	args_addarg("a","all","Do all things",ARGVAL_NONE);
	args_addarg("v",NULL,"Set verbosity level",ARGVAL_STRING);
	args_addarg(NULL,"yo-dawg","Yo dawg",ARGVAL_STRING);
	args_addarg("h","help","Display help",ARGVAL_NONE);
	// The following will be discarded
	args_addarg(NULL,NULL,"No names!",ARGVAL_NONE);

	if( (args_parse(argc,argv) != ARGRET_OK) &&
			(args_parsefile("./testconfig") != ARGRET_OK) )
		printf("Error occurred parsing command line.\n");
	else{
		ArgStr verbosity;
		ArgStr unnamed[3];

		if( args_check("a") )
			printf("Doing all things!\n");
		if( verbosity = args_getnamed("v") )
			printf("Verbosity set to %s.\n",verbosity);
		if( args_check("h") )
			args_print();
		if( unnamed[0] = args_getunnamed(1) )
			printf("First unnamed: %s.\n",unnamed[0]);
		if( unnamed[1] = args_getunnamed(2) )
			printf("Second unnamed: %s.\n",unnamed[1]);
		if( unnamed[2] = args_getunnamed(3) )
			printf("Third unnamed: %s.\n",unnamed[2]);
		args_unknown();
	}
	args_free();
}
#endif//ARGS_TEST

