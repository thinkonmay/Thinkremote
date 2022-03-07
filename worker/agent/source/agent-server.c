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
#include <win32-server.h>
#include <agent-port-forward.h>



#include <stdio.h>
#include <libsoup/soup.h>
#include <glib-2.0/glib/gstdio.h>
#include <libsoup/soup.h>

#ifdef G_OS_WIN32
#include <Windows.h>
#endif

#include <logging.h>
#include <json-handler.h>
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

#ifndef G_OS_WIN32
	SoupServer* server;
#else
	Win32Server* server;
#endif

	RemoteSession* remote_session;

	SoupWebsocketConnection* websocket;

	PortForward* portforward;
};




static void
handle_ping_thread(gpointer data)
{
	AgentServer* agent = (AgentServer*) data;
	static gboolean ping = TRUE;
	if(!ping)
	{
		ping = TRUE;
		return;
	}


	ping = FALSE;
	Sleep(30000);
	if(!ping)
	{
		restart_portforward(agent->portforward);
		ping = FALSE;
	}
	else
	{
		ping = TRUE:
	}
}


gboolean    
handle_message_server(gchar* path,
					  gchar* token,
                      GBytes* request_body,
                      gchar* response_body,
                      gpointer data)
{
	AgentServer* agent = (AgentServer*) data;
	
	if(!g_strcmp0(path,"/ping"))
	{
		g_thread_new("ping-thread",handle_ping_thread,data);
		return TRUE;
	}
	


	if(!g_strcmp0(path,"/Initialize")) {
		if(!validate_token(token))
			return FALSE;
		return session_initialize(agent);
	} else if(!g_strcmp0(path,"/Terminate")) {
		if(!validate_token(token))
			return FALSE;
		return session_terminate(data);
	}
}

void do_nothing() { }

void
development_environment_quit(ChildProcess* proc,
                            AgentServer* agent,
                            gpointer data)
{
    agent_finalize(agent);
}

void
restore_child_process(ChildProcess* proc,
                      AgentServer* agent,
                      gpointer data)
{
	create_new_child_process(childprocess_get_name(proc), 	
		do_nothing, do_nothing, 
		restore_child_process, agent, NULL);
}

void
development_agent(AgentServer* agent)
{
	gchar* ip = get_local_ip();

	GString* string = g_string_new("ws://");
	g_string_append(string,ip);
	g_string_append(string,":5000/Handshake");
	gchar* handshake = g_string_free(string,FALSE);

	string = g_string_new("Signalling.exe --urls=http://");
	g_string_append(string,ip);
	g_string_append(string,":5000");
	gchar* signalling_url = g_string_free(string,FALSE);

	gchar* remote_url = "remote.exe --urls=http://localhost:5000/";

	SetEnvironmentVariable("SIGNALLING",TEXT(handshake));
	create_new_child_process(signalling_url, 										do_nothing, do_nothing, restore_child_process, agent, NULL);
	create_new_child_process(remote_url, 											do_nothing, do_nothing, restore_child_process, agent, NULL);
	create_new_child_process("session-webrtc.exe 	--environment=development", 	do_nothing, do_nothing, restore_child_process, agent, NULL);
	create_new_child_process("remote-webrtc.exe 	--environment=development", 	do_nothing, do_nothing, do_nothing, agent, NULL);
}



AgentServer*
agent_new(gchar* token)
{
	AgentServer* agent = malloc(sizeof(AgentServer));
	memset(agent,0,sizeof(AgentServer));

	if(DEVELOPMENT_ENVIRONMENT)
	{
		development_agent(agent);
		goto run;
	}

	agent->portforward = init_portforward_service();
	agent->remote_session = intialize_remote_session_service();
	agent->socket = initialize_socket();

	if(!start_portforward(agent))
	{
		worker_log_output("Fail to start port-forward to cluster");
		goto fail;
	}

	// Always use window http server for window 
	// (libsoup server yield a bad performance)
#ifndef G_OS_WIN32
	agent->server = init_agent_server(agent,FALSE);
#else
	agent->server = init_window_server((ServerMessageHandle)handle_message_server,
		portforward_get_agent_instance_port(agent->portforward),agent);
#endif

	if(!agent->server)
	{
		worker_log_output("Fail to create agent server");
		goto fail;
	}


	
	register_with_managed_cluster(agent, agent->portforward, token);
run:
	agent->loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(agent->loop);
	return agent;
fail:
	return NULL;
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