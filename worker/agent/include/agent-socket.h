/**
 * @file agent-socket.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __SOCKET_H__
#define __SOCKET_H__


#include <glib.h>
#include <libsoup/soup.h>
#include <agent-server.h>













/**
 * @brief 
 * 
 * @param agent 
 */
void 									register_with_selfhosted_cluster	(AgentServer* agent);

/**
 * @brief 
 * 
 * @param agent 
 * @param agent_instance_port 
 * @param core_instance_port 
 */
void 									register_with_managed_cluster		(AgentServer* agent, 
                              												 gchar* agent_instance_port,
                              												 gchar* core_instance_port);
/**
 * @brief 
 * send message to cluster at a specific endpoint
 * @param object agent server
 * @param endpoint (enpoint present by string) (for ex: core/end)
 * @param message (Nullable) messsage body
 * @return gboolean 
 */
gboolean 								send_message_to_cluster				(AgentServer* object,
																			 gchar* endpoint,
																			 gchar* message);

/**
 * @brief 
 * initialize socket
 * @return Socket* pointer to socket instance
 */
Socket*                                 initialize_socket                   ();
#endif