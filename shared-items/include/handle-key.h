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

#include <human-interface-opcode.h>


#include <Windows.h>
#include <gst/gst.h>


void            handle_input_win32              (gchar* message,  
                                                GstElement* core);

void            handle_input_gtk                 (gchar* message,  
                                                GstElement* core);

void            handle_input_javascript         (gchar* message, 
                                                GstElement* core);


#endif