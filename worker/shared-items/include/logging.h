/**
 * @file logging.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __LOGGING_H__
#define __LOGGING_H__
#include <glib.h>



/**
 * @brief 
 * send all log to worker manager
 * @param text 
 */
void                worker_log_output           (gchar* text);


#endif