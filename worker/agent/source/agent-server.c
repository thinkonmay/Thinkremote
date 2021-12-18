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



#include <stdio.h>
#include <libsoup/soup.h>
#include <glib-2.0/glib/gstdio.h>
#include <libsoup/soup.h>
#ifdef G_OS_WIN32
#include <Windows.h>
#endif

#include <logging.h>
#include <error-code.h>
#include <message-form.h>
#include <global-var.h>
#include <token-validate.h>

/// <summary>
/// agent object 
/// </summary>
struct _AgentServer
{
	/// <summary>
	/// socket object
	/// </summary>
	Socket* socket;

	/// <summary>
	/// gmainloop will be run in agent process
	/// </summary>
	GMainLoop* loop;

	/// <summary>
	/// 
	/// </summary>
	SoupServer* server;

	/// <summary>
	/// 
	/// </summary>
	RemoteSession* remote_session;
};




static void
server_callback (SoupServer        *server,
                 SoupMessage	   *msg,
		 		 const char        *path,
                 GHashTable        *query,
				 SoupClientContext *ctx,
		 		 gpointer           user_data)
{
	char *file_path;
	SoupMessageHeadersIter iter;
	SoupMessageBody *request_body;
	const char *name, *value;
	AgentServer* agent = (AgentServer*)user_data;
	SoupURI* uri = soup_message_get_uri(msg);
	gchar* request_token;

	if(!g_strcmp0(uri->path,"/cluster/ping"))
	{
		gchar* response = "ping";
		soup_message_set_response(msg, "application/json",SOUP_MEMORY_COPY,response,strlen(response));
		soup_message_set_status(msg,SOUP_STATUS_OK);
		return;
	}
	else if(!g_strcmp0(uri->path,"/cluster/Shell")) {
		initialize_shell_session(agent,msg);
		return;
	}

	soup_message_headers_iter_init (&iter, msg->request_headers);
	while (soup_message_headers_iter_next (&iter, &name, &value))
	{
		if(!g_strcmp0(name,"Authorization"))
		{
			if(!validate_token(value))
			{
				msg->status_code = SOUP_STATUS_UNAUTHORIZED;
				return;
			}
		}
	}

	if(!g_strcmp0(uri->path,"/cluster/Initialize")) {

		msg->status_code = session_initialize(agent)? SOUP_STATUS_OK : SOUP_STATUS_BAD_REQUEST;
	}
	else if(!g_strcmp0(uri->path,"/cluster/Disconnect")) {
		msg->status_code = session_disconnect(agent)? SOUP_STATUS_OK : SOUP_STATUS_BAD_REQUEST;
	}
	else if(!g_strcmp0(uri->path,"/cluster/Reconnect")) {
		msg->status_code = session_reconnect(agent)? SOUP_STATUS_OK : SOUP_STATUS_BAD_REQUEST;
	}
	else if(!g_strcmp0(uri->path,"/cluster/Terminate")) {
		msg->status_code = session_terminate(agent)? SOUP_STATUS_OK : SOUP_STATUS_BAD_REQUEST;
	}
}







static SoupServer*
init_agent_server(AgentServer* agent)
{
	GError* error = NULL;
	SoupServer* server = soup_server_new(NULL);
	agent->server = server;

	soup_server_add_handler(agent->server,"/",
		(SoupServerCallback)server_callback,agent,NULL);

	gint port = atoi(AGENT_PORT);
	soup_server_listen_all(agent->server,port,0,&error);
	if(error){g_printerr(error->message); return;}
}


AgentServer*
agent_new()
{	
	GError* error = NULL;
	AgentServer* agent = malloc(sizeof(AgentServer));
	memset(agent,0,sizeof(AgentServer));

	agent->remote_session = intialize_remote_session_service();
	agent->socket = initialize_socket();
	agent->server = init_agent_server(agent);
	if(!agent->server){return;}
	
	register_with_host(agent);
	agent->loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(agent->loop);
	return agent;
}


void
agent_finalize(AgentServer* self)
{
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

void
agent_set_remote_session(AgentServer* self, 
						 RemoteSession* session)
{
	self->remote_session = session;
}

