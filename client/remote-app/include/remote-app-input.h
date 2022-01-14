#ifndef __REMOTE_APP_INPUT_H__
#define __REMOTE_APP_INPUT_H__
#include <glib-2.0/glib.h>
#include <gst/gst.h>
#include <remote-app-type.h>





#ifdef G_OS_WIN32
#include <windows.h>

/**
 * @brief 
 * 
 * @param isup 
 * @param app 
 */
void                handle_window_wheel             (gint isup,
                                                     RemoteApp* app);



/**
 * @brief 
 * 
 * @param hwnd 
 * @param message 
 * @param wParam 
 * @param lParam 
 */
void                handle_message_window_proc      (HWND hwnd, 
                                                    UINT message, 
                                                    WPARAM wParam, 
                                                    LPARAM lParam);

/**
 * @brief 
 * 
 * @param app 
 */
void                trigger_capture_input_event     (RemoteApp* app);

/**
 * @brief 
 * 
 * @return InputHandler* 
 */
InputHandler*       init_input_capture_system       (RemoteApp* app);



/**
 * @brief 
 * 
 * @param app 
 */
void                reset_key                       (RemoteApp* app);

/**
 * @brief 
 * 
 * @param app 
 */
void                reset_mouse                     (RemoteApp* app);







/**
 * @brief 
 * 
 * @param mouse_code 
 * @param delta_X 
 * @param delta_Y 
 * @param app 
 */
void                handle_window_mouse_relative    (gint mouse_code,
                                                    gint delta_X,
                                                    gint delta_Y,
                                                    RemoteApp* app);


/**
 * @brief 
 * enable or disable key capture 
 */
void                toggle_key_capturing            (RemoteApp* app, 
                                                    gboolean is_true);
#else

/**
 * @brief 
 * handle navigation event from gstreamer 
 * @param event 
 * @param core 
 * @return gboolean 
 */
gboolean            handle_navigator                (GstEvent *event,
                                                     RemoteApp* core);

#endif
#endif