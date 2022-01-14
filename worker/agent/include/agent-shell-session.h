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
 * 
 * @param agent 
 * @param input 
 * @param output 
 * @return gboolean 
 */
gboolean 			initialize_shell_session_from_byte			(AgentServer* agent,
																 GBytes* input,
																 gchar* output);
#endif


