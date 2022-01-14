/**
 * @file agent-port-forward.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2022-01-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __AGENT_PORT_FORWARD_H__
#define __AGENT_PORT_FORWARD_H__

#include <glib.h>
#include <agent-type.h>


/**
 * @brief 
 * 
 * @param agent 
 * @return gpointer 
 */
PortForward*            start_portforward                       (AgentServer* agent);


/**
 * @brief 
 * 
 * @return PortForward* 
 */
PortForward*            init_portforward_service                ();

/**
 * @brief 
 * 
 */
void                    describe_portforward                    (PortForward* port_forward, 
                                                                 gchar* agent_instance_port,
                                                                 gchar* core_instance_port);


/**
 * @brief 
 * 
 * @param port 
 * @return gchar* 
 */
gchar*                  portforward_get_agent_instance_port     (PortForward *port);






#endif