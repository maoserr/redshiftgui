/**\file		randr.h
 * \author		Mao Yu,Jon Lund Steffensen
 * \date		Modified: Saturday, July 10, 2010
 * \brief		Randr interface
 */

#ifndef _REDSHIFT_RANDR_H
#define _REDSHIFT_RANDR_H
#ifdef ENABLE_RANDR

/**\brief Initialize Randr */
int randr_init(int screen_num, int crtc_num);

/**\brief Frees Randr */
int randr_free(void);

/**\brief Restores saved gamma ramps */
void randr_restore(void);

/**\brief Sets the temperature using Randr */
int randr_set_temperature(int temp, gamma_s gamma);

/**\brief Retrieves the temperature
 * \bug Sometimes Randr returns 6500K even when it's not
 */
int randr_get_temperature(void);

/**\brief loads functions into methods structure */
int randr_load_funcs(gamma_method_s *method);

#endif /*ENABLE_RANDR*/
#endif /* ! _REDSHIFT_RANDR_H */
