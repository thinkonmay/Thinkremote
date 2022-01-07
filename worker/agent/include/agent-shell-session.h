/**
 * @file agent-shell-session.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __AGENT_SHELL_SESSION_H__
#define __AGENT_SHELL_SESSION_H__

#include <glib.h>
#include <libsoup/soup.h>
#include <agent-server.h>








/**
 * @brief 
 * initlize shell session with message from cluster manager
 * @param agent 
 * @param message soup message from cluster manager
 */
void				initialize_shell_session					(AgentServer* agent,
                    										     SoupMessage* message);

/**
 * @brief 
 * initialize shell session without soup message
 * @param agent 
 * @param data 
 * @return gchar* 
 */
gchar* 				initialize_shell_session_websocket			(AgentServer* agent, 
                                   								 gchar* data);

#endif


