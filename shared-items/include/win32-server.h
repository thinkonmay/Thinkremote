/**
 * @file win32-server.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2022-01-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef __AGENT_WIN32_SERVER__
#define __AGENT_WIN32_SERVER__


/**
 * @brief 
 * 
 */
typedef struct _Win32Server         Win32Server;


typedef gboolean    (*ServerMessageHandle)  (gchar* path,
                                             gchar* token,
                                             GBytes* request_body,
                                             gchar* response_body,
                                             gpointer data);

Win32Server* init_window_server             (ServerMessageHandle handle,
                                             gpointer data);


#endif