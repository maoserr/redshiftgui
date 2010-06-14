/**\file		argparser.h
 * \author		Mao Yu (http://www.mao-yu.com)
 * \date		Created: Friday, May 28, 2010
 * \date		Updated: Friday, June 04, 2010
 * \brief		Parses command line argument given argc and argv.
 * \details		Argument parsing functionality.
 * Instructions:
 *	- Call 'args_addarg' to add valid options.
 *		this function will copy the short, long, and help string.
 *	- Call 'args_parse' and pass in 'argc' and 'argv' to parse.
 *	- Use 'args_check','args_getunnamed','args_getnamed' to detect
 *		arguments.
 *	- Optionally use 'args_print' to show valid arguments list.
 *	- Optionally use 'args_unknown' to show unknown arguments.
 *		(this function will return ARGBOOL_TRUE if there are unknowns)
 *	- Call 'args_free' to free all resources.
 *
 * Notes:
 *	Short arguments should be 1 or 2 letters.
 *	Long arguments needs to be 3 letters or longer.
 *
 *	See bottom of "argparser.c" for test case.
 *
 *	This is public domain code.
 */

#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__

typedef char*		ArgStr;
typedef char		ArgChar;
typedef enum{
	ARGBOOL_FALSE,
	ARGBOOL_TRUE
} ArgBool;

/**Possible error codes.*/
typedef enum{
	ARGRET_OK,
	ARGRET_MEM_ERROR,
	ARGRET_NO_NAMES,
	ARGRET_INVALID_SNAME,
	ARGRET_INVALID_LNAME,
	ARGRET_INVALID_HELP,
	ARGRET_INVALID_FILE,
	ARGRET_FILE_LINE_TOO_LONG,
	ARGRET_FILE_READ_ERROR
} ArgReturn;

/**Possible types of arguments.*/
typedef enum {
	ARGVAL_NONE,
	ARGVAL_STRING,
} ArgValType;

/**\brief Add valid arguments for parsing.
 * \param shortname A short 1 or 2 letter flag (do NOT prefix '-').
 * \param longname A long name, more than 2 letters (do NOT prefix '--').
 * \param helpstring Helpstring shown when args_print used.
 * \param valtype Type of value this argument accepts.
 * \return ArgReturn error code.*/
ArgReturn args_addarg(
		const ArgStr shortname,
		const ArgStr longname,
		const ArgStr helpstring,
		const ArgValType valtype);

/**\brief Parses argc and argv for options.
 * \param argc Number of arguments.
 * \param argv Array of strings containing arguments.
 * \return ArgReturn error code.*/
ArgReturn args_parse(int argc, char *argv[]);

/**\brief Parses a config file for options.
 * \details To override command line options, call this function first,
 * otherwise calling args_parse first will override any config file options.
 * \param filename Name of config file.*/
ArgReturn args_parsefile(ArgStr filename);

/**\brief Checks for an option.
 * \param name Short or long name.
 * \return (ArgBool) True or False if argument flag is set.*/
ArgBool args_check(const ArgStr name);

/**\brief Retrieves and unnamed option based on index.
 * \details If you exec "executable option1 option2 -flag1"
 * "option1" and "option2" are unnamed options because they are not prefixed
 * by "-" or "--"
 * \param index 1 is the first option, 2 is the second.
 * \return String containing the option.*/
ArgStr args_getunnamed(const int index);

/**\brief Retrieves named option based on short or long name.
 * \param name Short or long name.
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
