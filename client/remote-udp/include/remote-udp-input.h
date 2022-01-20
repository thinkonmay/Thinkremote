#ifndef __REMOTE_APP_INPUT_H__
#define __REMOTE_APP_INPUT_H__
#include <glib-2.0/glib.h>
#include <gst/gst.h>
#include <remote-udp-type.h>





#ifdef G_OS_WIN32
#include <windows.h>

/**
 * @brief 
 * 
 * @param isup 
 * @param app 
 */
void                handle_window_wheel             (gint isup,
                                                     RemoteUdp* app);



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
void                trigger_capture_input_event     (RemoteUdp* app);

/**
 * @brief 
 * 
 * @return InputHandler* 
 */
InputHandler*       init_input_capture_system       (RemoteUdp* app);



/**
 * @brief 
 * 
 * @param app 
 */
void                reset_key                       (RemoteUdp* app);

/**
 * @brief 
 * 
 * @param app 
 */
void                reset_mouse                     (RemoteUdp* app);




/**
 * @brief 
 * 
 */
void                setup_input_endpoint            (InputHandler* handler,
                                                    gchar* input_ip,
                                                    gchar* input_port);



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
                                                    RemoteUdp* app);


/**
 * @brief 
 * enable or disable key capture 
 */
void                toggle_key_capturing            (RemoteUdp* app, 
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
                                                     RemoteUdp* core);

#endif
#endif