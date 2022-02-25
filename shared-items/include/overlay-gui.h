/**
 * @file remote-webrtc-gui.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __REMOTE_APP_GUI_H__
#define __REMOTE_APP_GUI_H__
#include <gst/gst.h>
#include <glib-2.0/glib.h>
#include <capture-key.h>






/**
 * @brief 
 * GUI is a datastructure wrap around creation of remote window and win32 input handling
 */
typedef struct 		_GUI 				                    GUI;







/**
 * @brief Set the up window object
 * 
 * @param gui 
 */
GUI*                init_remote_app_gui                     (gpointer app,
                                                             Shortcut* shortcuts,
                                                             HIDHandleFunction event);






/**
 * @brief Set the up video overlay object
 * 
 * @param videosink 
 * @param app 
 * @return gpointer 
 */
gpointer            setup_video_overlay                     (GUI* gui,
                                                             GstCaps* caps,
                                                             GstElement* videosink, 
                                                             GstElement* pipeline);

/**
 * @brief 
 * 
 * @return gboolean 
 */
gboolean            is_hover_window                         ();

#endif