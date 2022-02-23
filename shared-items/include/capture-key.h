/**
 * @file capture-key.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2022-02-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __CAPTURE_KEY_H__
#define __CAPTURE_KEY_H__

#include <glib-2.0/glib.h>
#include <enum.h>

/**
 * @brief 
 * hid handler is a datastructure that wrap around handling of 
 */
typedef struct          _InputHandler                         InputHandler;




/**
 * @brief 
 * 
 */
typedef void            (*HIDHandleFunction)                    (gchar* message,
                                                                 gpointer data);

/**
 * @brief 
 * 
 */
typedef void            (*ShortcutHandleFuntion)                (gpointer data);

/**
 * @brief 
 * 
 */
typedef struct _Shortcut
{
    ShortcutOpcode opcode;

    gint key_list[10];

    gpointer data;

    ShortcutHandleFuntion function;

    gboolean active;
}Shortcut;

/**
 * @brief 
 * 
 * @param mouse_code 
 * @param delta_X 
 * @param delta_Y 
 */
void                handle_window_mouse_relative                (gint mouse_code,
                                                                gint delta_X,
                                                                gint delta_Y);

/**
 * @brief 
 * 
 * @param isup 
 * @param app 
 */
void                handle_window_wheel                         (gint isup);


#ifdef G_OS_WIN32
#include <Windows.h>
/**
 * @brief 
 * 
 * @param hwnd 
 * @param message 
 * @param wParam 
 * @param lParam 
 */
void                handle_message_window_proc                  (HWND hwnd, 
                                                                UINT message, 
                                                                WPARAM wParam, 
                                                                LPARAM lParam);
#endif










/**
 * @brief 
 * 
 * @return InputHandler* 
 */
InputHandler*       init_input_capture_system                   (HIDHandleFunction function,
                                                                 Shortcut* shortcuts,
                                                                 gpointer data);


/**
 * @brief 
 * 
 */
void                trigger_hotkey_by_opcode                    (ShortcutOpcode opcode);
#endif