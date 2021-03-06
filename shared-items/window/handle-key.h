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
#define __HANDLE_KEY_H__

#include <glib-2.0/glib.h>
#include <gst/gst.h>
#include <shortcut.h>

typedef struct  _HIDHandler                     HIDHandler;

typedef void    (*MousePositionFeedbackFunc)    (gpointer data,
                                                 gchar* feedback);


/**
 * @brief 
 * 
 * @param message 
 */
void            handle_input_win32              (gchar* message,
                                                 MousePositionFeedbackFunc feedback,
                                                 gpointer data);

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
HIDHandler*     activate_hid_handler            (GstElement* capture, 
                                                 Shortcut* shortcuts);

/**
 * @brief 
 * 
 */
void            deactivate_hid_handler          (HIDHandler* handler);










#endif