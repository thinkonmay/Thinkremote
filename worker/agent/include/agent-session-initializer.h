/**
 * @file agent-session-initializer.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __AGENT_SESSION_INITIALIZER_H__ 
#define __AGENT_SESSION_INITIALIZER_H__ 

#include <glib.h>
#include <agent-server.h>
#include <agent-type.h>



/**
 * @brief 
 * send message to session core 
 * @param self 
 * @param buffer 
 * @return gboolean 
 */
gboolean			send_message_to_core							(AgentServer* self,
																	 gchar* buffer);

/**
 * @brief 
 * initialize session by create session core process
 * @param object 
 * @return gboolean 
 */
gboolean			session_initialize   							(AgentServer* object);

/**
 * @brief 
 * terminate session by force stop session core process
 * @param object 
 * @return gboolean 
 */
gboolean 			session_terminate   							(AgentServer* object);


/**
 * @brief 
 * initlaize RemoteSession instance
 * @return RemoteSession* 
 */
RemoteSession*		intialize_remote_session_service				();
																	 
#endif