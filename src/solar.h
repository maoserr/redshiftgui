/**\file		solar.h
 * \author		Mao Yu,Jon Lund Steffensen
 * \date		Saturday, July 10, 2010
 * \brief		Procedures to calculate solar elevation
 * \details
 * Ported from javascript code by U.S. Department of Commerce,
 * National Oceanic & Atmospheric Administration:
 * http://www.srrb.noaa.gov/highlights/sunrise/calcdetails.html
 * It is based on equations from "Astronomical Algorithms" by
 * Jean Meeus.
 */

#ifndef _SOLAR_H
#define _SOLAR_H

#include "time.h"

/**\brief Model of atmospheric refraction near horizon (in degrees). */
#define SOLAR_ATM_REFRAC  0.833

/**\brief Uh.... */
#define SOLAR_ASTRO_TWILIGHT_ELEV   -18.0f
/**\brief Uh.... */
#define SOLAR_NAUT_TWILIGHT_ELEV    -12.0f
/**\brief Uh.... */
#define SOLAR_CIVIL_TWILIGHT_ELEV    -6.0f
/**\brief Uh.... */
#define SOLAR_DAYTIME_ELEV           (0.0f - SOLAR_ATM_REFRAC)

/**\brief Converts degrees to radians */
#define RAD(x)  ((x)*(M_PI/180))
/**\brief Converts radians to degrees */
#define DEG(x)  ((x)*(180/M_PI))

/**\brief Solar times */
typedef enum {
	SOLAR_TIME_NOON = 0,		/**< Uh... */
	SOLAR_TIME_MIDNIGHT,		/**< Uh... */
	SOLAR_TIME_ASTRO_DAWN,		/**< Uh... */
	SOLAR_TIME_NAUT_DAWN,		/**< Uh... */
	SOLAR_TIME_CIVIL_DAWN,		/**< Uh... */
	SOLAR_TIME_SUNRISE,			/**< Uh... */
	SOLAR_TIME_SUNSET,			/**< Uh... */
	SOLAR_TIME_CIVIL_DUSK,		/**< Uh... */
	SOLAR_TIME_NAUT_DUSK,		/**< Uh... */
	SOLAR_TIME_ASTRO_DUSK,		/**< Uh... */
	SOLAR_TIME_MAX				/**< Uh... */
} solar_time_t;


/**\brief Calculates solar elevation given date, latitude, and longitude */
double solar_elevation(double date, double lat, double lon);

/**\brief Solar table initialization function, no idea */
void solar_table_fill(double date, double lat, double lon, double *table);

#endif /* ! _SOLAR_H */
