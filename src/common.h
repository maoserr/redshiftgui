/**\file		common.h
 * \author		Mao Yu
 * \brief		Common definitions.
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef _WIN32
# define WINVER  0x0500
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(s) gettext(s)
#else
# define _(s) s
#endif

#if _MSC_VER
# define INLINE __inline
#else
# define INLINE inline
#endif

#define _(X)		X
#define LOG(X,...)	printf(__VA_ARGS__)
#define LOGERR	1
#define LOGWARN	2
#define LOGINFO	3
#define LOGVERBOSE 4

// Main program return codes
#define RET_MAIN_OK 0
#define RET_MAIN_ERR -1

// Function return codes
#define RET_FUN_FAILED	0
#define RET_FUN_SUCCESS	1

#define LONGEST_PATH 2048
#ifndef _WIN32
 #define PATH_SEP '/'
 #define RSG_RCFILE ".redshiftgrc"
#else
 #define PATH_SEP '\\'
 #define RSG_RCFILE "redshiftg.ini"
#endif

INLINE float MAX(float x,float y)
{return (x<y)?x:y;}

INLINE float MIN(float x, float y)
{return (x>y)?x:y;}

#endif//__COMMON_H__
