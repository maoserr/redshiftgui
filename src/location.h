/**\file	location.h
 * \author	Mao Yu
 * \date	Thursday, July 08, 2010
 * \brief	Location manipulation functions
 */

#ifndef __LOCATION_H__
#define __LOCATION_H__

/**\brief Method to look up lat/lon */
typedef enum{
	LOCMETHOD_GEOCODE_HOSTIP,	/**< Use hostip.info to look up lat/lon */
	LOCMETHOD_ADDRESS_LOOKUP,	/**< Use Google maps to look up lat/lon */
	LOCMETHOD_MANUAL			/**< Not used */
} loc_method_t;

/**\brief Geocode from www.hostip.info*/
int location_geocode_hostip(float *lat,float *lon,char *city,int bsize);

/**\brief Geocode from geobytes*/
int location_geocode_geobytes(float *lat,float *lon,char *city,int bsize);

/**\brief Look up location from address (from Google maps api)*/
int location_address_lookup(char *address,float *lat,float *lon,
		char *city,int bsize);

/**\brief Initialize location lookup*/
int location_init(void);

/**\brief Shut down location lookup*/
int location_end(void);

#endif//__LOCATION_H__
