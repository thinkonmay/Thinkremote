/**
 * @file agent-server.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <agent-server.h>
#include <agent-type.h>
#include <agent-session-initializer.h>
#include <agent-socket.h>
#include <agent-device.h>
#include <agent-shell-session.h>
#include <agent-device.h>
#include <agent-child-process.h>
#include <agent-win32-server.h>
#include <agent-port-forward.h>



#include <stdio.h>
#include <libsoup/soup.h>
#include <glib-2.0/glib/gstdio.h>
#include <libsoup/soup.h>
#ifdef G_OS_WIN32
#include <Windows.h>
#endif

#include <logging.h>
#include <message-form.h>
#include <global-var.h>
#include <token-validate.h>

/**
 * @brief 
 * 
 */
struct _AgentServer
{
	Socket* socket;

	GMainLoop* loop;

	SoupServer* server;

	RemoteSession* remote_session;

	SoupWebsocketConnection* websocket;

	PortForward* portforward;
};





gboolean    
handle_message_server(gchar* path,
					  gchar* token,
                      GBytes* request_body,
                      gchar* response_body,
                      gpointer data)
{
	AgentServer* agent = (AgentServer*) data;

	if(!g_strcmp0(path,"/ping"))
		return TRUE;
	


	if(!g_strcmp0(path,"/Initialize")) {
		if(!validate_token(token))
			return FALSE;
		return session_initialize(agent);
	} else if(!g_strcmp0(path,"/Terminate")) {
		if(!validate_token(token))
			return FALSE;
		return session_terminate(data);
	} else if(!g_strcmp0(path,"/Shell")) {
		return initialize_shell_session_from_byte(agent,request_body,response_body);
	} 
}






AgentServer*
agent_new(gboolean self_host)
{	
	GError* error = NULL;
	AgentServer* agent = malloc(sizeof(AgentServer));
	memset(agent,0,sizeof(AgentServer));

	agent->loop = g_main_loop_new(NULL, FALSE);
	agent->portforward = init_portforward_service();
	agent->remote_session = intialize_remote_session_service();
	agent->socket = initialize_socket();

#ifndef G_OS_WIN32
	agent->server = init_agent_server(agent,self_host);
#else
	agent->server = init_window_server(handle_message_server,agent);
#endif

	if(!agent->server){return;}

	if(self_host) {
		register_with_selfhosted_cluster(agent,self_host);
	} else {
		PortForward* port = start_portforward(agent);

		register_with_managed_cluster(agent,
			portforward_get_agent_instance_port(port), NULL);
	}

	
	g_main_loop_run(agent->loop);
	return agent;
}


void
agent_finalize(AgentServer* self)
{
	session_terminate(self);
	if (self->loop)
		g_main_loop_quit(self->loop);

#ifdef G_OS_WIN32
	ExitProcess(0);
#endif
}







/*START get-set function*/
Socket*
agent_get_socket(AgentServer* self)
{
	return self->socket;
}

void
agent_set_socket(AgentServer* self, Socket* socket)
{
	self->socket = socket;
}



void
agent_set_main_loop(AgentServer* self,
	GMainLoop* loop)
{
	self->loop = loop;
}

GMainLoop*
agent_get_main_loop(AgentServer* self)
{
	return self->loop;
}


RemoteSession*
agent_get_remote_session(AgentServer* self)
{
	return self->remote_session;
}

PortForward*
agent_get_portforward(AgentServer* self)
{
	return self->portforward;
}

void
agent_set_remote_session(AgentServer* self, 
						 RemoteSession* session)
{
	self->remote_session = session;
}

