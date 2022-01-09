#ifndef __AGENT_SERVER_H__
#define __AGENT_SERVER_H__
#include <glib.h>


#include <agent-type.h>
#include <agent-device.h>
 





/**
 * @brief 
 * finalize agent server and terminate process
 * @param object 
 */
void											agent_finalize						(AgentServer* object);


/**
 * @brief 
 * iniitalize agent server and mainloop
 * @return AgentServer* 
 */
AgentServer*									agent_new							(gboolean port_forward);

/**
 * @brief 
 * start file transfer
 * @param server_commmand 
 */
void											agent_start_file_transfer			(gchar* server_commmand);



/**
 * @brief 
 * get socket from agent server
 * @param self 
 * @return Socket* 
 */
Socket*											agent_get_socket					(AgentServer* self);





/**
 * @brief 
 * get remote session from agent server
 * @param self  agent server
 * @return RemoteSession* remote session instance
 */
RemoteSession*									agent_get_remote_session			(AgentServer* self);



/*END get-set function for agent object*/


#endif // !__AGENT_OBJECT__