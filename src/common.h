/**\file		common.h
 * \author		Mao Yu
 * \date		Modified: Saturday, July 10, 2010
 * \brief		Common definitions.
 */
#ifndef __COMMON_H__
#define __COMMON_H__

/*@ignore@*/
/*@-nestcomment@*/
#ifdef _WIN32
# define WINVER  0x0500
# define WIN32_LEAN_AND_MEAN
# ifdef _MSC_VER
#  define _USE_MATH_DEFINES
#  define _CRT_SECURE_NO_WARNINGS
#  define snprintf sprintf_s
# endif
# include <windows.h>
#endif
/*@=nestcomment@*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <time.h>
#include "thirdparty/logger.h"
#ifndef _WIN32
# ifndef S_SPLINT_S
#  include <unistd.h>
# endif
#endif
/*@end@*/
#ifndef M_PI
# define M_PI 3.14159
#endif

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(s) gettext(s)
#else
/**\brief Wrapper for strings that could be translated */
# define _(s) s
#endif

/*@ignore@*/
// Logging macros
#if 0
#define LOG(LVL,...) log_log(LVL,__FILE__,__FUNCTION__ ,__LINE__,__VA_ARGS__)
#else
/**\brief Logging macro, outputs function name */
#define LOG(LVL,...) (void)log_log(LVL,NULL,__FUNCTION__,0,__VA_ARGS__)
#endif
/*@end@*/
/**\brief Log level error */
#define LOGERR		1
/**\brief Log level warning */
#define LOGWARN		2
/**\brief Log level info */
#define LOGINFO		3
/**\brief Log level verbose */
#define LOGVERBOSE	4

/**\brief Function failed */
#define RET_FUN_FAILED	0
/**\brief Function success */
#define RET_FUN_SUCCESS	1

// Configuration file strings
/**\brief Size of path buffers */
#define LONGEST_PATH 2048
#ifndef _WIN32
/**\brief Platform dependent path separator */
 #define PATH_SEP '/'
/**\brief Configuration file name */
 #define RSG_RCFILE ".redshiftgrc"
#else
 #define PATH_SEP '\\'
 #define RSG_RCFILE "redshiftg.ini"
#endif

// Stringify
#define STR(X) X

// Math macros
#ifndef MIN
/**\brief Minimum of two values */
# define MIN(x,y)  ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
/**\brief Maximum of two values */
# define MAX(x,y)  ((x) > (y) ? (x) : (y))
#endif

// MSVC round
#ifdef _MSC_VER
#define round(X) floor((X) + 0.5)
#endif

#ifndef _WIN32
/**\brief Platform dependent sleep wrapper */
# define SLEEP(X) usleep(1000*X)
#else
# define SLEEP(X) Sleep(X)
#endif

#endif//__COMMON_H__

