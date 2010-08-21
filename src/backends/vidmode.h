/**\file		vidmode.h
 * \author		Mao Yu,Jon Lund Steffensen
 * \date		Modified: Saturday, July 10, 2010
 * \brief		VidMode interface
 */

#ifndef _REDSHIFT_VIDMODE_H
#define _REDSHIFT_VIDMODE_H
#ifdef ENABLE_VIDMODE

/**\brief Initialize VidMode */
int vidmode_init(int screen_num,int crtc_num);

/**\brief Frees VidMode state */
int vidmode_free(void);

/**\brief Restores saved gamma ramps */
void vidmode_restore(void);

/**\brief Sets temperature using VidMode */
int vidmode_set_temperature(int temp, gamma_s gamma);

/**\brief Retrieves temperature using VidMode */
int vidmode_get_temperature(void);

/**\brief Loads VidMode functions into methods structure */
int vidmode_load_funcs(gamma_method_s *method);

#endif /*ENABLE_VIDMODE*/
#endif /* ! _REDSHIFT_VIDMODE_H */
