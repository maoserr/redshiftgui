/**\file		options.h
 * \author		Mao Yu
 * \brief		Options.
 */
#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include "common.h"
#include "gamma.h"

/**\brief Load default values for all fields.
 */
void opt_set_defaults(void);

/**\brief Sets the CRTC to apply adjustment to.
 * \param val integer value of CRTC
 */
SetRet opt_set_crtc(int val);

/**\brief Sets additional gamma adjustment.
 * \param r red value
 * \param g green value
 * \param b blue value
 */
SetRet opt_set_gamma(float r, float g, float b);

/**\brief Parses a string containing gamma adjustment
 * \param val string containing text in the form of R:G:B
 */
SetRet opt_parse_gamma(char *val);

/**\brief Sets the location with latitude/longitude
 * \param lat latitude
 * \param lon longitude
 */
SetRet opt_set_location(float lat, float lon);

/**\brief Parses a string containing location
 * \param val string containing text in the form of LAT:LON
 */
SetRet opt_parse_location(char *val);

/**\brief Sets the method of gamma adjustment
 * \param method gamma_method_t argument
 */
SetRet opt_set_method(gamma_method_t method);

/**\brief Parses a string of the method
 * \param val string containing either 
 *		"randr" (or "RANDR"),
 *		"vidmode" (or "VidMode"),
 *	or	"wingdi" (or "WinGDI")
 */
SetRet opt_parse_method(char *val);

/**\brief Sets one shot mode (adjust and then exit)
 * \param onoff set to 1 to enable
 */
SetRet opt_set_oneshot(int onoff);

/**\brief Sets transition speed
 * \param tpersec temperature per second, defaults to 100k/s
 */
SetRet opt_set_transpeed(int tpersec);

/**\brief Sets the screen to apply adjustment to.
 * \param val integer value of the screen.
 */
SetRet opt_set_screen(int val);

/**\brief Sets the temperatures for day/night
 * \param tday temperature for daytime (defaults to DEFAULT_DAY_TEMP)
 * \param tnight temperature for nighttime (defaults to DEFAULT_NIGHT_TEMP)
 */
SetRet opt_set_temperatures(int tday, int tnight);

/**\brief Parse the temperatures from string
 * \param val String containing text in the form of "DAY:NIGHT"
 */
SetRet opt_parse_temperatures(char *val);

/**\brief Sets the verbosity of the logging level.
 * \param val verbosity
 */
SetRet opt_set_verbose(int val);

/**\brief Retrieves CRTC */
int opt_get_crtc(void);

/**\brief Retrieves Gamma */
gamma_s opt_get_gamma(void);

/**\brief Retrieves latitude */
float opt_get_lat(void);

/**\brief Retrieves longitude */
float opt_get_lon(void);

/**\brief Retrieves method */
gamma_method_t opt_get_method(void);

/**\brief Retrieves oneshot mode */
int opt_get_oneshot(void);

/**\brief Retrieves transition speed */
int opt_get_trans_speed(void);

/**\brief Retrieves screen */
int opt_get_screen(void);

/**\brief Retrieves day temperature */
int opt_get_temp_day(void);

/**\brief Retrieves night temperature */
int opt_get_temp_night(void);

/**\brief Retrieves verbosity level */
int opt_get_verbosity(void);

#endif//__OPTIONS_H__
