/**\file		argparser.h
 * \brief		argparser include file.
 * \mainpage Argument parser
 * \author		Mao Yu (http://www.mao-yu.com)
 * \date		Created: Friday, May 28, 2010
 * \date		Updated: Saturday, June 19, 2010
 * \par Installation:
 *	-# Downloads:
 *		- <a href="http://github.com/maoserr/C-library/tarball/master">tarball</a>.
 *		- <a href="http://github.com/maoserr/C-library/zipball/master">zip</a>.
 *	-# Add header and source file to your project
 *		- argparser.h
 *		- argparser.c
 * \par Usage:
 *	- Call args_addarg() to add valid options.
 *		this function will copy the short, long, and help string.
 *	- Call args_parse() and pass in \b argc and \b argv to parse.
 *	- Optionally use args_parsefile() to parse a config file.
 *	- Use args_check(),args_getunnamed(),args_getnamed() to detect
 *		arguments.
 *	- Optionally use args_print() to show valid arguments list.
 *	- Optionally use args_unknown() to show unknown arguments.
 *		(this function will return ARGBOOL_TRUE if there are unknowns)
 *	- Call args_free() to free all resources.
 *
 * \note
 * \par
 *	-# Short arguments should be 1 or 2 letters.
 *		- Example: '-o', or config file format 'o'
 *	-# Long arguments needs to be longer than 3 letters and shorter than 10.
 *		- Example: '-one', or config file format 'one'
 *	-# Arguments that accept values are space delimited in command line, but '='
 *	delimited in config file
 *		- Example '-type moose', or config file format 'type=moose'
 *	-# Unnamed arguments are arguments that are not prefixed with '-' and are
 *	not values for named arguments.
 *	-# Unknown arguments are arguments prefixed with '-' but are not validly
 *	added arguments.
 *
 *	\par An example program
 *	\code
 * #include <stdio.h>
 * #include "argparser.h"
 *
 * int main( int argc, char *argv[] ){
 * 	args_addarg("a","all","Do all things",ARGVAL_NONE);
 * 	args_addarg("v",NULL,"Set verbosity level",ARGVAL_STRING);
 * 	args_addarg(NULL,"yo-dawg","Yo dawg",ARGVAL_STRING);
 * 	args_addarg("h","help","Display help",ARGVAL_NONE);
 * 	// The following will be discarded
 * 	args_addarg(NULL,NULL,"No names!",ARGVAL_NONE);
 *
 * 	if( (args_parse(argc,argv) != ARGRET_OK) &&
 * 			(args_parsefile("./testconfig") != ARGRET_OK) )
 * 		printf("Error occurred parsing command line.\n");
 * 	else{
 * 		ArgStr verbosity;
 * 		ArgStr unnamed[3];
 *
 * 		if( args_check("a") )
 * 			printf("Doing all things!\n");
 * 		if( verbosity = args_getnamed("v") )
 * 			printf("Verbosity set to %s.\n",verbosity);
 * 		if( args_check("h") )
 * 			args_print();
 * 		if( unnamed[0] = args_getunnamed(1) )
 * 			printf("First unnamed: %s.\n",unnamed[0]);
 * 		if( unnamed[1] = args_getunnamed(2) )
 * 			printf("Second unnamed: %s.\n",unnamed[1]);
 * 		if( unnamed[2] = args_getunnamed(3) )
 * 			printf("Third unnamed: %s.\n",unnamed[2]);
 * 		args_unknown();
 * 	}
 * 	args_free();
 * }
 *	\endcode
 * Running this code with this command:
\verbatim
argparser -a -v DEBUG hello you good people
\endverbatim
 * Will show this output:
\verbatim
Doing all things!
Verbosity set to DEBUG.
First unnamed: hello.
Second unnamed: you.
Third unnamed: good.
\endverbatim
 *	See \ref argparser.h "here" for full API documentation.
 *
 * \par License
 * This is public domain code.
 */

#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__

/**\brief String type */
typedef char*		ArgStr;
/**\brief Character type */
typedef char		ArgChar;

/** Logger boolean type*/
typedef enum{
	ARGBOOL_FALSE,	/*!< */
	ARGBOOL_TRUE	/*!< */
} ArgBool;

/**Possible error codes.*/
typedef enum{
	ARGRET_OK,				/*!< Success */
	ARGRET_MEM_ERROR,		/*!< Memory allocation error */
	ARGRET_NO_NAMES,		/*!< No short or long names provided */
	ARGRET_INVALID_SNAME,	/*!< Invalid short name */
	ARGRET_INVALID_LNAME,	/*!< Invalid long name */
	ARGRET_INVALID_HELP,	/*!< Invalid help string */
	ARGRET_INVALID_FILE,	/*!< Invalid file */
	ARGRET_FILE_LINE_TOO_LONG,	/*!< Line in file exceeded buffer length */
	ARGRET_FILE_READ_ERROR		/*!< File read error */
} ArgReturn;

/**Possible types of arguments.*/
typedef enum {
	ARGVAL_NONE,
	ARGVAL_STRING,
} ArgValType;

/**\brief Add valid arguments for parsing.
 * \param[in] shortname A short 1 or 2 letter flag (do NOT prefix '-').
 * \param[in] longname A long name, more than 2 letters (do NOT prefix '--').
 * \param[in] helpstring Helpstring shown when args_print used.
 * \param[in] valtype Type of value this argument accepts.
 * \return ArgReturn error code.*/
ArgReturn args_addarg(
		const ArgStr shortname,
		const ArgStr longname,
		const ArgStr helpstring,
		const ArgValType valtype);

/**\brief Parses argc and argv for options.
 * \param[in] argc Number of arguments.
 * \param[in] argv Array of strings containing arguments.
 * \return ArgReturn error code.*/
ArgReturn args_parse(int argc, char *argv[]);

/**\brief Parses a config file for options.
 * \details To override command line options, call this function first,
 * otherwise calling args_parse first will override any config file options.
 * \param[in] filename Name of config file.*/
ArgReturn args_parsefile(ArgStr filename);

/**\brief Checks for an option.
 * \param[in] name Short or long name.
 * \return (ArgBool) True or False if argument flag is set.*/
ArgBool args_check(const ArgStr name);

/**\brief Retrieves and unnamed option based on index.
 * \details If you exec "executable option1 option2 -flag1"
 * "option1" and "option2" are unnamed options because they are not prefixed
 * by "-" or "--"
 * \param[in] index 1 is the first option, 2 is the second.
 * \return String containing the option.*/
ArgStr args_getunnamed(const int index);

/**\brief Retrieves named option based on short or long name.
 * \param[in] name Short or long name.
 * \return String containing the option.*/
ArgStr args_getnamed(const ArgStr name);

/**\brief Prints a list of options and help string.*/
void args_print(void);

/**\brief Prints a list of unknown options.
 * \return (ArgBool) True if unknowns exist.*/
ArgBool args_unknown(void);

/**\brief Frees all resources.*/
void args_free(void);

#endif//__ARG_PARSER_H__
