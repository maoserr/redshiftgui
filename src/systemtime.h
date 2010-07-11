/**\file		systemtime.h
 * \author		Mao Yu,Jon Lund Steffensen
 * \date		Modified: Saturday, July 10, 2010
 * \brief		Gets system time in a platform independent way
 */

#ifndef _REDSHIFT_SYSTEMTIME_H
#define _REDSHIFT_SYSTEMTIME_H

#ifndef _WIN32
# include <time.h>
#endif

/**\brief Retrieves system time for solar elevation calculation */
int systemtime_get_time(double *now);

#endif /* ! _REDSHIFT_SYSTEMTIME_H */
