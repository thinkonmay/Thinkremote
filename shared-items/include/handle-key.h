/**
 * @file shortcut.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2022-02-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __HANDLE_KEY_H__
#define __SHORTCUT_H__

#include <glib-2.0/glib.h>
#include <json-glib/json-glib.h>

#include <enum.h>
#include <capture-key.h>


#include <Windows.h>
#include <gst/gst.h>


/**
 * @brief 
 * 
 * @param message 
 */
void            handle_input_win32              (gchar* message);

/**
 * @brief 
 * 
 * @param message 
 */
void            handle_input_gtk                (gchar* message);

/**
 * @brief 
 * 
 * @param message 
 */
void            handle_input_javascript         (gchar* message);

/**
 * @brief 
 * 
 * @param capture 
 * @param shortcuts 
 */
void            activate_hid_handler            (GstElement* capture, 
                                                Shortcut* shortcuts);

/**
 * @brief 
 * 
 */
void            deactivate_hid_handler          ();
#endif