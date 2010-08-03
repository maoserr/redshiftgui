/**\file		gamma.h
 * \author		Mao Yu,Jon Lund Steffensen
 * \date		Modified: Saturday, July 10, 2010
 * \brief		Gamma/temperature adjustment functions.
 */

#ifndef __GAMMA_H__
#define __GAMMA_H__

#include <stdint.h>

/**\brief Minimum gamma value */
#define MIN_GAMMA	0.1
/**\brief Maximum gamma value */
#define MAX_GAMMA	10.0
/**\brief Minimum temperature */
#define MIN_TEMP	3400
/**\brief Maximum temperature */
#define MAX_TEMP	7000

/**\brief Default brightness */
#define DEFAULT_BRIGHTNESS	1.0f
/**\brief Default daytime temperature limit */
#define DEFAULT_DAY_TEMP	6500
/**\brief Default nighttime temperature limit */
#define DEFAULT_NIGHT_TEMP	3600
/**\brief Default gamma values */
#define DEFAULT_GAMMA		1.0

/**\brief gamma structure */
typedef struct{
	/**\brief Red */
	float r;
	/**\brief Green */
	float g;
	/**\brief Blue */
	float b;
} gamma_s;

/**\brief gamma ramp structure */
typedef struct{
	/**\brief Pointer to all ramps */
	uint16_t *all;
	/**\brief Pointer to red ramp */
	uint16_t *r;
	/**\brief Pointer to green ramp */
	uint16_t *g;
	/**\brief Pointer to blue ramp */
	uint16_t *b;
	/**\brief Size of the ramps */
	int size;
} gamma_ramp_s;

/**\brief Gamma method functions */
typedef struct{
	/**\brief Function to initialize method */
	int (*func_init)(int screen_num,int crtc_num);
	/**\brief Function to shutdown method */
	int (*func_end)(void);
	/**\brief Function to set the temperature */
	int (*func_set_temp)(int temp, gamma_s gamma);
	/**\brief Function to get the temperature */
	int (*func_get_temp)(void);
	/**\brief Method name. */
	char *name;
} gamma_method_s;

/**\brief Enum of gamma adjustment methods */
typedef enum {
	GAMMA_METHOD_NONE,		/**< No method defined */
	GAMMA_METHOD_AUTO,		/**< Try all methods */
	GAMMA_METHOD_RANDR,		/**< Linux RANDR */
	GAMMA_METHOD_VIDMODE,	/**< Linux VidMode */
	GAMMA_METHOD_WINGDI,	/**< Win32 GDI */
	GAMMA_METHOD_MAX		/**< Tracks the highest value */
} gamma_method_t;

/**\brief Re-allocates ramps if size changed, otherwise return pointer to ramp */
gamma_ramp_s gamma_get_ramps(int size);

/**\brief Updates gamma ramp structure */
gamma_ramp_s gamma_ramp_fill(int size,int temp);

/**\brief Retrieves current active method name */
char *gamma_get_active_method_name(void);

/**\brief Retrieves method name by id */
char *gamma_get_method_name(gamma_method_t method);

/**\brief Find the temperature based on red:blue ratio */
int gamma_find_temp(float ratio);

/**\brief Load methods available */
int gamma_load_methods(void);

/**\brief Looks up method by name */
gamma_method_t gamma_lookup_method(char *name);

/**\brief Initialize gamma changing method. */
gamma_method_t gamma_init_method(int screen_num, int crtc_num,
		gamma_method_t method);

/**\brief Restores gamma ramps to default values */
int gamma_state_restore(void);

/**\brief Free the state associated with the appropriate adjustment method. */
int gamma_state_free(void);

/**\brief Calculate temperature based on elevation. */
int gamma_calc_temp(double elevation, int temp_day, int temp_night);

/**\brief Calculates the target temperature for now */
int gamma_calc_curr_target_temp(float lat, float lon,
		int temp_day, int temp_night);

/**\brief Sets the temperature */
int gamma_state_set_temperature(int temp, gamma_s gamma);

/**\brief Retrieves current temperature */
int gamma_state_get_temperature(void);

#endif//__GAMMA_H__
