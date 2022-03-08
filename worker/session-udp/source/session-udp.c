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
#include <session-udp-pipeline.h>
#include <session-udp.h>
#include <session-udp-type.h>

#include <logging.h>
#include <json-handler.h>
#include <global-var.h>
#include <token-validate.h>
#include <constant.h>
#include <enum.h>
#include <remote-config.h>


#include <glib.h>

#ifdef G_OS_WIN32
#include <handle-key.h>
#include <win32-server.h>
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
	 * StreamConfig of the stream
	 */
	UdpEndpoint* audio;

	/**
	 * @brief 
	 * StreamConfig of the stream
	 */
	UdpEndpoint* video;

	DeviceType device;

	CoreEngine engine;
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
#ifdef G_OS_WIN32	
	gchar* audio_port = GetEnvironmentVariableWithKey("AUDIO_PORT");
	gchar* audio_host = GetEnvironmentVariableWithKey("AUDIO_HOST");
	gchar* video_port = GetEnvironmentVariableWithKey("VIDEO_PORT");
	gchar* video_host = GetEnvironmentVariableWithKey("VIDEO_HOST");

	self->audio = udp_endpoint_new(audio_port,audio_host);
	self->video = udp_endpoint_new(video_port,video_host);
#endif
	worker_log_output("session core setup done");
}


/**
 * @brief 
 * handle message from hid datachannel and send to window
 * @param dc 
 * @param message 
 * @param core 
 */
static void
on_hid_input(gchar* message,
			 SessionUdp* udp)
{
    Pipeline* pipeline = session_core_get_pipeline(udp);
    if(DEVELOPMENT_ENVIRONMENT)
        g_print("%s\n",message);

#ifdef G_OS_WIN32
    if(udp->device == WEB_APP && udp->engine == CHROME)
        handle_input_javascript(message);
    else if(udp->device == WINDOW_APP && udp->engine == GSTREAMER)
        handle_input_win32(message,(MousePositionFeedbackFunc)NULL,udp);
#endif
}

void
on_setup_message(gchar* text,
				 SessionUdp* agent)
{
    JsonParser* parser = json_parser_new();
    JsonObject* object = get_json_object_from_string(text,NULL,parser);
    if(!object)
        goto free;
    agent->device =  json_object_get_int_member(object,"Device");
    agent->engine =  json_object_get_int_member(object,"Engine");
free:
    g_object_unref(parser);
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
	gchar* text = g_bytes_get_data(request_body,NULL);

	if(!g_strcmp0(path,"/hid")) 
		on_hid_input(text,agent);
	else if(!g_strcmp0(path,"/setup")) 
		on_setup_message(text,agent);
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

		if(infor_message->status_code != SOUP_STATUS_OK)
			session_core_finalize(core,NULL);

#ifdef G_OS_WIN32
		Sleep(1000);
#else
		sleep(1000);
#endif
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
		on_hid_input(msg->request_body->data,user_data);
		msg->status_code = SOUP_STATUS_OK;
	}
}

SessionUdp*
session_core_initialize()
{
	SessionUdp* core = malloc(sizeof(SessionUdp));

	core->qoe =					qoe_initialize();
	core->pipe =				pipeline_initialize();
	core->loop =				g_main_loop_new(NULL, FALSE);

#ifdef G_OS_WIN32
	core->server = 				init_window_server((ServerMessageHandle)handle_message_server,"6003",core);
#endif

	session_core_setup_session(core);

	if(!DEVELOPMENT_ENVIRONMENT)
		g_thread_new("Sync",(GThreadFunc) session_core_sync_state_with_cluster,core);

	setup_pipeline(core);
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



StreamConfig*
session_core_get_qoe(SessionUdp* self)
{
	return self->qoe;
}

UdpEndpoint*
session_core_get_audio_endpoint(SessionUdp* self)
{
	return self->audio;
}

UdpEndpoint*
session_core_get_video_endpoint(SessionUdp* self)
{
	return self->video;
}


SignallingHub*
session_core_get_signalling_hub(SessionUdp* core)
{
	return core->signalling;
}
#ifndef G_OS_WIN32
Display*
session_core_display_interface(SessionUdp* self)
{
	return self->x_display;

}
#endif