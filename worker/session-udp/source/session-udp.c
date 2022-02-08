/**
 * @file session-udp.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <session-udp-remote-config.h>
#include <session-udp-pipeline.h>
#include <session-udp-human-interface.h>
#include <session-udp.h>
#include <session-udp-type.h>

#include <module-code.h>
#include <opcode.h>
#include <logging.h>
#include <message-form.h>
#include <global-var.h>
#include <token-validate.h>
#include <development.h>
#include <human-interface-opcode.h>
#include <win32-server.h>


#include <glib.h>

#ifdef G_OS_WIN32
#include <Windows.h>

#else
#include <Xlib.h>
#endif


struct _SessionUdp
{
#ifndef G_OS_WIN32
	/**
	 * @brief 
	 * Soup server for receiving ping from cluster manager
	 */
	SoupServer* server;
#else
	/**
	 * @brief 
	 * 
	 */
	Win32Server* server;
#endif

	/**
	 * @brief 
	 * pipeline of the stream
	 */
	Pipeline* pipe;

	/**
	 * @brief 
	 * webrtchub to communicate with client
	 */
	HumanInterface* hub;

	/**
	 * @brief 
	 * mainloop run throughout the stream
	 */
	GMainLoop* loop;

	/**
	 * @brief 
	 * signalling hub for connection with signalling server
	 */
	SignallingHub* signalling;

	/**
	 * @brief 
	 * StreamConfig of the stream
	 */
	StreamConfig* qoe;

	/**
	 * @brief 
	 * 
	 */
	DeviceType peer_device;

	/**
	 * @brief 
	 * 
	 */
	CoreEngine peer_engine;

#ifndef G_OS_WIN32
	Display* x_display
#endif
};





/**
 * @brief 
 * callback function used to handle soup message from client,
 * those message will only come from agent and cluster manager 
 * @param server session core soup server
 * @param msg message that need to handle
 * @param path 
 * @param query 
 * @param user_data pointer to session core
 */
void
server_callback (SoupServer        *server,
                 SoupMessage	   *msg,
		 		 const char        *path,
                 GHashTable        *query,
				 SoupClientContext *ctx,
		 		 gpointer           user_data);





/**
 * @brief 
 * setup session by request input from database with worker token given by agent
 * @param self session core
 */
static void
session_core_setup_session(SessionUdp* self)
{
	JsonParser* token_parser = json_parser_new();
	gchar* remote_token;


	if(DEVELOPMENT_ENVIRONMENT)
	{
		remote_token = DEFAULT_CORE_TOKEN;
	}
	else
	{
		const char* http_aliases[] = { "http", NULL };
		SoupSession* http_session = soup_session_new_with_options(
				SOUP_SESSION_SSL_STRICT, FALSE,
				SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
				SOUP_SESSION_HTTPS_ALIASES, http_aliases, NULL);

		GString* token_url= g_string_new(CLUSTER_URL);
		g_string_append(token_url,"/worker/session/token");
		gchar* token_str = g_string_free(token_url,FALSE);

		worker_log_output("getting remote token from server\n");
		worker_log_output(token_str);


		SoupMessage* token_message = soup_message_new(SOUP_METHOD_POST,token_str);
		gchar* buffer = "null";
		soup_message_set_request(token_message,"application/json", SOUP_MEMORY_STATIC, "null", 4);

		worker_log_output("registering with device token\n");
		worker_log_output(DEVICE_TOKEN);

		soup_message_headers_append(token_message->request_headers,
			"Authorization",DEVICE_TOKEN);
		soup_session_send_message(http_session,token_message);

		if(token_message->status_code == SOUP_STATUS_OK )
		{
			GError* error = NULL;
			JsonObject* json_infor = get_json_object_from_string(token_message->response_body->data,error,token_parser);
			remote_token = json_object_get_string_member(json_infor,"token");
		}
		else 
		{
			g_printerr ("got response code %d\n",token_message->status_code);
			GError* error = malloc(sizeof(GError));
			error->message = "fail to get session information";
			session_core_finalize(self,error);
			return;
		}
	}


	if (DEVELOPMENT_ENVIRONMENT)
	{

		qoe_setup(self->qoe,
					1920,
					1080,
					OPUS_ENC,
					CODEC_H265,
					DEVELOPMENT_DEFAULT_BITRATE);
		
		self->peer_device = WINDOW_APP;
	}
	else
	{
		worker_log_output("got remote token\n");
		worker_log_output(remote_token);

		const char* https_aliases[] = { "https", NULL };
		SoupSession* https_session = soup_session_new_with_options(
				SOUP_SESSION_SSL_STRICT, FALSE,
				SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
				SOUP_SESSION_HTTPS_ALIASES, https_aliases, NULL);

		GString* infor_url = g_string_new(SESSION_URL);
		g_string_append(infor_url,	"?token=");
		g_string_append(infor_url,	remote_token);


		gchar* infor_str = g_string_free(infor_url,FALSE);
		SoupMessage* infor_message = soup_message_new(SOUP_METHOD_GET,infor_str);

		soup_session_send_message(https_session,infor_message);


		if(infor_message->status_code == SOUP_STATUS_OK)
		{
			GError* error = NULL;
			JsonParser* parser = json_parser_new();
			JsonObject* json_infor = get_json_object_from_string(infor_message->response_body->data,error,parser);

			qoe_setup(self->qoe,
						json_object_get_int_member(json_infor,"screenwidth"),
						json_object_get_int_member(json_infor,"screenheight"),
						json_object_get_int_member(json_infor,"audiocodec"),
						json_object_get_int_member(json_infor,"videocodec"),
						json_object_get_int_member(json_infor,"mode"));
			

			self->peer_device =	json_object_get_int_member(json_infor,"clientdevice");
			self->peer_engine =	json_object_get_int_member(json_infor,"clientengine");
			g_object_unref(parser);
		}
		else 
		{
			GError* error = malloc(sizeof(GError));
			error->message = "fail to get session information";
			session_core_finalize(self,error);
			return;
		}
	}
	
		
	worker_log_output("session core setup done");
}





#ifdef G_OS_WIN32
gboolean    
handle_message_server(gchar* path,
					  gchar* token,
                      GBytes* request_body,
                      gchar* response_body,
                      gpointer data)
{
	SessionUdp* agent = (SessionUdp*) data;
	if(!g_strcmp0(path,"/hid")) {
		gchar* text = g_bytes_get_data(request_body,NULL);
		on_human_interface_message(text,agent);
		return TRUE;
	}
	return TRUE;
}


#endif

/**
 * @brief 
 * initialize session core with message handler
 * @param core 
 * @return SoupServer* 
 */
static SoupServer*
init_session_core_server(SessionUdp* core)
{
	GError* error = NULL;
	SoupServer* server = soup_server_new(NULL);

	soup_server_add_handler(server,
		"/",server_callback,core,NULL);

	soup_server_listen_all(server,2250,0,&error);
	if(error){g_printerr(error->message); return;}
	return server;
}




gpointer
session_core_sync_state_with_cluster(gpointer user_data)
{
#ifdef G_OS_WIN32
        Sleep(3000);
#else
        sleep(3000);
#endif
	SessionUdp* core = (SessionUdp*)user_data;
	const char* https_aliases[] = { "https", NULL };
	SoupSession* https_session = soup_session_new_with_options(
			SOUP_SESSION_SSL_STRICT, FALSE,
			SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
			SOUP_SESSION_HTTPS_ALIASES, https_aliases, NULL);
	
	GString* infor_url = g_string_new(CLUSTER_URL);
	g_string_append(infor_url,"/worker/session/continue");
	gchar* infor_url_str = g_string_free(infor_url,FALSE);
	while (TRUE)
	{
		SoupMessage* infor_message = soup_message_new(SOUP_METHOD_POST,infor_url_str);
		gchar* buffer = "null";
		soup_message_set_request(infor_message, "application/json", SOUP_MEMORY_STATIC, buffer, strlen(buffer));
		soup_message_headers_append(infor_message->request_headers, "Authorization",DEVICE_TOKEN);

		soup_session_send_message(https_session,infor_message);

		if(infor_message->status_code == SOUP_STATUS_OK)
		{
#ifdef G_OS_WIN32
			Sleep(1000);
#else
			sleep(1000);
#endif
		}
		else
		{
			session_core_finalize(core,NULL);
		}
	}
}

void
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
	SoupURI* uri = soup_message_get_uri(msg);
	if(!g_strcmp0(uri->path,"/ping"))
	{
		soup_message_set_response(msg, "application/json",SOUP_MEMORY_STATIC,"null",4);
		msg->status_code = SOUP_STATUS_OK;
		return;
	} else if(!g_strcmp0(uri->path,"/hid"))
	{
		on_human_interface_message(msg->request_body->data,user_data);
		msg->status_code = SOUP_STATUS_OK;
	}
}

SessionUdp*
session_core_initialize()
{
	UdpEndpoint endpoint = {6001,6002,"192.168.1.6","192.168.1.6"};
	worker_log_output("Session core process started");
	SessionUdp* core = malloc(sizeof(SessionUdp));

#ifndef USE_LIBSOUP 
	core->server = 				init_session_core_server(core);
#else
	core->server = 				init_window_server((ServerMessageHandle)handle_message_server,"6003",core);
#endif

	core->hub =					human_interface_initialize();
	core->qoe =					qoe_initialize();
	core->pipe =				pipeline_initialize();
	core->loop =				g_main_loop_new(NULL, FALSE);

	session_core_setup_session(core);

	if(!DEVELOPMENT_ENVIRONMENT)
		g_thread_new("Sync",(GThreadFunc) session_core_sync_state_with_cluster,core);

	setup_pipeline(core,endpoint);
	g_main_loop_run(core->loop);
	return core;	
}











void
session_core_finalize(SessionUdp* self, 
					  GError* error)
{
	if(error)
		worker_log_output(error->message);

	if(self->loop)
		g_main_loop_unref(self->loop);

#ifdef G_OS_WIN32
	ExitProcess(0);
#endif

}












Pipeline*
session_core_get_pipeline(SessionUdp* self)
{
	return self->pipe;
}

HumanInterface*
session_core_get_rtc_hub(SessionUdp* self)
{
	return self->hub;
}


StreamConfig*
session_core_get_qoe(SessionUdp* self)
{
	return self->qoe;
}


SignallingHub*
session_core_get_signalling_hub(SessionUdp* core)
{
	return core->signalling;
}

DeviceType		
session_core_get_client_device(SessionUdp* self)
{
	return self->peer_device;
}

CoreEngine
session_core_get_client_engine(SessionUdp* self)
{
	return self->peer_engine;
}

#ifndef G_OS_WIN32
Display*
session_core_display_interface(SessionUdp* self)
{
	return self->x_display;

}
#endif