/**
 * @file global-var.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef __GLOBAL_VAR_H__
#define __GLOBAL_VAR_H__
#include <glib-2.0/glib.h>


const gchar*                      token ;
const gchar*                      cluster_ip ; 
const gchar*                      device_token ; 
const gchar*                      cluster_token ; 



/**
 * @brief 
 * clueter manager ip address
 */
#define			CLUSTER_URL 				cluster_ip 

/**
 * @brief 
 * worker token to communication with other module
 */
#define			CLUSTER_TOKEN               cluster_token 

/**
 * @brief 
 * worker token to communication with other module
 */
#define			DEVICE_TOKEN                device_token 

/**
 * @brief 
 * setup defaul value
 */
void            thinkremote_application_init   ();


#endif