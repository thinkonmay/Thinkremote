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
session_core_setup_session(SessionUdp* self,
							gchar* data)
{
	g_print("%s\n",data);

	gchar* audio_port, *audio_host, *video_port, *video_host;
#ifdef G_OS_WIN32	
	audio_port = GetEnvironmentVariableWithKey("AUDIO_PORT");
	audio_host = GetEnvironmentVariableWithKey("AUDIO_HOST");
	video_port = GetEnvironmentVariableWithKey("VIDEO_PORT");
	video_host = GetEnvironmentVariableWithKey("VIDEO_HOST");

	if(audio_port && audio_host && video_port && video_host) 
		goto done;
#endif

	JsonParser* parser = json_parser_new();
	JsonObject* json = get_json_object_from_string(data,NULL,parser);

	audio_port = json_object_get_string_member(json,"AUDIO_PORT");
	audio_host = json_object_get_string_member(json,"AUDIO_HOST");
	video_port = json_object_get_string_member(json,"VIDEO_PORT");
	video_host = json_object_get_string_member(json,"VIDEO_HOST");

    self->device =  json_object_get_int_member(json,"Device");
    self->engine =  json_object_get_int_member(json,"Engine");
done:
	self->audio = udp_endpoint_new(audio_port,audio_host);
	self->video = udp_endpoint_new(video_port,video_host);

	worker_log_output("session core setup done");
	setup_pipeline(self,json);
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
server_callback (SoupServer        *server,
                 SoupMessage	   *msg,
		 		 const char        *path,
                 GHashTable        *query,
				 SoupClientContext *ctx,
		 		 gpointer           user_data)
{
	SessionUdp* udp = (SessionUdp*)user_data;

	char *file_path;
	SoupMessageHeadersIter iter;
	SoupMessageBody *request_body;
	const char *name, *value;
	SoupURI* uri = soup_message_get_uri(msg);
	gchar* request_token;

	



	if(!g_strcmp0(uri->path,"/Initialize")) 
		session_core_setup_session(udp,msg->request_body->data);
	else if(!g_strcmp0(uri->path,"/hid")) 
		on_hid_input(msg->request_body->data,udp);
	
	msg->status_code = SOUP_STATUS_OK;
	
}

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

	soup_server_listen_all(server,3567,0,&error);
	if(error){g_printerr(error->message); return;}
	return server;
}







SessionUdp*
session_core_initialize()
{
	SessionUdp* core = malloc(sizeof(SessionUdp));

	core->pipe =				pipeline_initialize();
	core->loop =				g_main_loop_new(NULL, FALSE);

#ifdef G_OS_WIN32
	core->server = 				init_session_core_server(core);
#endif

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

