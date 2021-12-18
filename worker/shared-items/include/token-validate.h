/**
 * @file token-validate.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __TOKEN_VALIDATE_H__
#define __TOKEN_VALIDATE_H__

#include <glib-2.0/glib.h>
#include <libsoup/soup.h>


/**
 * @brief 
 * validate token by sending it to cluster manager
 * @param token 
 * @return gboolean 
 */
gboolean                validate_token          (gchar* token);


#endif