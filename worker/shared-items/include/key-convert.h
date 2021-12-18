/**
 * @file key-convert.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __KEY_CONVERT_H__
#define __KEY_CONVERT_H__
#include <glib.h>


/**
 * @brief 
 * convert key from javascript standard to window 
 * @param keycode 
 * @return unsigned short 
 */
unsigned short
convert_javascript_key_to_window_key(gchar* keycode);


#endif