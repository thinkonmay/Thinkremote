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
#include <glib.h>



gchar*                   get_thinkremote_cluster_ip();

gchar*                   get_thinkremote_device_token();

gchar*                   get_thinkremote_cluster_token();

gboolean                 get_environment();

gboolean                 is_localhost_env();



/**
 * @brief 
 * 
 */
#define         DEVELOPMENT_ENVIRONMENT     get_environment()

/**
 * @brief 
 * 
 */
#define         LOCALHOST                   is_localhost_env()

/**
 * @brief 
 * clueter manager ip address
 */
#define			CLUSTER_URL 				get_thinkremote_cluster_ip() 

/**
 * @brief 
 * worker token to communication with other module
 */
#define			CLUSTER_TOKEN               get_thinkremote_cluster_token()

/**
 * @brief 
 * worker token to communication with other module
 */
#define			DEVICE_TOKEN                get_thinkremote_device_token()


/**
 * @brief 
 * 
 * @param environment 
 * @param cluster_url 
 * @param cluster_token 
 * @param device_token 
 */
void            thinkremote_application_init   (gchar* environment,
                                                gchar* cluster_url,
                                                gchar* cluster_token,
                                                gchar* device_token);




/**
 * @brief 
 * 
 * @param device_token 
 */
void            update_device_token             (gchar* device_token);

#endif