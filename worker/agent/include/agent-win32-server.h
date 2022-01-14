/**
 * @file agent-win32-server.h
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

#include <agent-type.h>



Win32Server* init_window_server             (ServerMessageHandle handle,
                                            AgentServer* agent);


#endif