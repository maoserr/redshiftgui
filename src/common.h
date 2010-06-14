/**\file		common.h
 * \author		Mao Yu
 * \brief		Common definitions.
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#if _MSC_VER
#define INLINE __inline
#else
#define INLINE inline
#endif

#define _(X)		X
#define LOG(X,...)	printf(__VA_ARGS__)
#define LOGERR	1
#define LOGWARN	2
#define LOGINFO	3

/// Setting options return values
typedef enum {
	SET_ERROR=0,
	SET_OK=1
} SetRet;

#endif//__COMMON_H__
