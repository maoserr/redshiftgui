/**\file		netutils.h
 * \author		Mao Yu
 * \date		Modified: Wednesday, August 11, 2010
 * \brief		Handles retrieving information from the net.
 */

#ifndef __NETUTILS_H__
#define __NETUTILS_H__

/**\brief Downloads url content to a new buffer, you must free buffer */
/*@null@*/ char *download2buffer(char url[]);

/**\brief Escapes special characters in URL (makes a new buffer you must free) */
/*@null@*/ char *escape_url(const char url[]);

/**\brief Parses a string given starting and ending tags, you must supply buffer
 * \return strlen of copied string (maybe truncated if buffer too small).
 */
int parse_tag_str(char content[],char start_tag[],char end_tag[],
		/*@unique@*/ char *buffer,int bsize);

/**\brief Parses a float given starting tag */
float parse_tag_float(char content[],char start[]);

/**\brief Initialize cURL */
int net_init(void);

/**\brief Unloads cURL */
int net_end(void);

#endif//__NETUTILS_H__
