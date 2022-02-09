


#include <glib.h>
#include <agent-server.h>
#include <agent-type.h>
#include <agent-libsoup-server.h>
#include <agent-session-initializer.h>
#include <agent-port-forward.h>

#include <libsoup/soup.h>

#include <global-var.h>

static void
server_ping (SoupServer        *server,
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
	soup_message_set_http_version(msg, SOUP_HTTP_1_0);
	SoupURI* uri = soup_message_get_uri(msg);
	gchar* request_token;

	if(!g_strcmp0(uri->path,"/ping")) {
		soup_message_set_status(msg,SOUP_STATUS_OK);
		return;
	} else if(!g_strcmp0(uri->path,"/Shell")) {
		// initialize_shell_session(agent,msg);
		soup_message_set_status(msg,SOUP_STATUS_OK);
		return;
	} 
}

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


	if(!g_strcmp0(uri->path,"/Initialize")) {
		msg->status_code = session_initialize(agent)? SOUP_STATUS_OK : SOUP_STATUS_BAD_REQUEST;
	} else if(!g_strcmp0(uri->path,"/Terminate")) {
		msg->status_code = session_terminate(agent)? SOUP_STATUS_OK : SOUP_STATUS_BAD_REQUEST;
	}
}


SoupServer*
init_agent_server(AgentServer* agent,
				  gboolean self_host)
{
	GError* error = NULL;
	SoupServer* server = soup_server_new(NULL);

	soup_server_add_handler(server,"/Initialize",
		(SoupServerCallback)server_callback,agent,NULL);

	soup_server_add_handler(server,"/Terminate",
		(SoupServerCallback)server_callback,agent,NULL);

	soup_server_add_handler(server,"/PortDescribe",
		(SoupServerCallback)server_callback,agent,NULL);

	soup_server_add_handler(server,"/ping",
		(SoupServerCallback)server_ping,agent,NULL);


	gint port = atoi(portforward_get_agent_instance_port(agent_get_portforward(agent)));
	if(self_host) {
		soup_server_listen_all(server,port,0,&error);
	} else {
		soup_server_listen_local(server,port,0,&error);
	}
	
	if(error){g_printerr(error->message); return NULL;}
    return server;
}
