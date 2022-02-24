/**
 * @file session-webrtc.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <session-webrtc-signalling.h>
#include <session-webrtc-pipeline.h>
#include <session-webrtc-data-channel.h>
#include <session-webrtc.h>
#include <session-webrtc-type.h>
#include <session-webrtc-qos.h>

#include <logging.h>
#include <json-handler.h>
#include <remote-config.h>
#include <global-var.h>
#include <token-validate.h>

#include <constant.h>
#include <environment.h>
#include <enum.h>


#include <glib.h>
#include <stdio.h>

#ifdef G_OS_WIN32
#include <Windows.h>

#else
#include <Xlib.h>
#endif


struct _SessionCore
{
	/**
	 * @brief 
	 * pipeline of the stream
	 */
	Pipeline* pipe;

	/**
	 * @brief 
	 * webrtchub to communicate with client
	 */
	WebRTCHub* hub;

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
};



void
session_development_setup(SessionCore* self)
{
	gchar* signalling = DEVELOPMENT_SIGNALLING_URL;
	if(!signalling)
	{
		gchar ip [100] = {0};
		g_print("target computer ip: ");
		scanf("%s",ip);

		GString* string = g_string_new("ws://");
		g_string_append(string,ip);
		g_string_append(string,":5000/Handshake");
		signalling = g_string_free(string,NULL);
	}

	JsonArray* array = json_array_new();
	json_array_add_string_element(array,DEFAULT_STUN);

#ifdef DEFAULT_TURN
	gchar* turn =	DEFAULT_TURN;
#else
	gchar* turn =	" ";
#endif

	signalling_hub_setup(self->signalling,
				turn,
				signalling,
				array,
				DEFAULT_CORE_TOKEN);

	qoe_setup(self->qoe,
				1920,
				1080,
				OPUS_ENC,
				CODEC_H265,
				ULTRA_HIGH_CONST);
}




/**
 * @brief 
 * setup session by request input from database with worker token given by agent
 * @param self session core
 */
static void
session_core_setup_session(SessionCore* self)
{
	if(DEVELOPMENT_ENVIRONMENT)
	{
		session_development_setup(self);
		return;
	}

	gchar* remote_token;
	{
		gchar* buffer = "null";
		const char* http_aliases[] = { "http", NULL };
		SoupSession* http_session = soup_session_new_with_options(
				SOUP_SESSION_SSL_STRICT, FALSE,
				SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
				SOUP_SESSION_HTTPS_ALIASES, http_aliases, NULL);

		GString* token_url= g_string_new(CLUSTER_URL);
		g_string_append(token_url,"/worker/session/token");
		gchar* token_str = g_string_free(token_url,FALSE);


		SoupMessage* message = soup_message_new(SOUP_METHOD_POST,token_str);
		soup_message_set_request(message,"application/json", SOUP_MEMORY_STATIC, buffer, strlen(buffer));
		soup_message_headers_append(message->request_headers, "Authorization",DEVICE_TOKEN);
		soup_session_send_message(http_session,message);

		if(message->status_code != SOUP_STATUS_OK )
		{
			worker_log_output("got response code %d when fetch remote token\n",message->status_code);
			session_core_finalize(self,NULL);
		}

		GError* error = NULL;
		JsonParser* token_parser = json_parser_new();
		JsonObject* json_infor = get_json_object_from_string(message->response_body->data,error,token_parser);
		remote_token = json_object_get_string_member(json_infor,"token");

		worker_log_output("got remote token\n");
	}

	{
		const char* https_aliases[] = { "https", NULL };
		SoupSession* https_session = soup_session_new_with_options(
				SOUP_SESSION_SSL_STRICT, FALSE,
				SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
				SOUP_SESSION_HTTPS_ALIASES, https_aliases, NULL);

		GString* infor_url = g_string_new(SESSION_URL);
		g_string_append(infor_url,	"?token=");
		g_string_append(infor_url,	remote_token);
		gchar* infor_str = g_string_free(infor_url,FALSE);


		SoupMessage* message = soup_message_new(SOUP_METHOD_GET,infor_str);
		soup_session_send_message(https_session,message);


		if(message->status_code != SOUP_STATUS_OK)
		{
			worker_log_output("got response code %d when fetch session information\n",message->status_code);
			session_core_finalize(self,NULL);
		}

		GError* error = NULL;
		JsonParser* parser = json_parser_new();
		JsonObject* json_infor = get_json_object_from_string(message->response_body->data,error,parser);


		signalling_hub_setup(self->signalling,
			json_object_get_string_member(json_infor,"turn"),
			json_object_get_string_member(json_infor,"signallingurl"),
			json_object_get_array_member(json_infor,"stuns"),
			remote_token);

		qoe_setup(self->qoe,
					json_object_get_int_member(json_infor,"screenwidth"),
					json_object_get_int_member(json_infor,"screenheight"),
					json_object_get_int_member(json_infor,"audiocodec"),
					json_object_get_int_member(json_infor,"videocodec"),
					json_object_get_int_member(json_infor,"mode"));
		
		g_object_unref(parser);
	}
	
		
	worker_log_output("session core setup done");
}









gpointer
session_core_sync_state_with_cluster(gpointer user_data)
{
	if(DEVELOPMENT_ENVIRONMENT)
		return;
#ifdef G_OS_WIN32
        Sleep(3000);
#else
        sleep(3000);
#endif
	SessionCore* core = (SessionCore*)user_data;

	gchar* buffer = "null";
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
		SoupMessage* message = soup_message_new(SOUP_METHOD_POST,infor_url_str);
		soup_message_set_request(message, "application/json", SOUP_MEMORY_STATIC, buffer, strlen(buffer));
		soup_message_headers_append(message->request_headers, "Authorization",DEVICE_TOKEN);
		soup_session_send_message(https_session,message);

		if(message->status_code != SOUP_STATUS_OK)
			session_core_finalize(core,NULL);

#ifdef G_OS_WIN32
		Sleep(1000);
#else
		sleep(1000);
#endif
	}
}


SessionCore*
session_core_initialize()
{
	worker_log_output("Session core process started");
	SessionCore* core = malloc(sizeof(SessionCore));

	core->hub =					webrtchub_initialize();
	core->signalling =			signalling_hub_initialize(core);
	core->qoe =					qoe_initialize();
	core->pipe =				pipeline_initialize();
	core->loop =				g_main_loop_new(NULL, FALSE);

	session_core_setup_session(core);
	signalling_connect(core);

	g_thread_new("Sync",(GThreadFunc)
		session_core_sync_state_with_cluster,core);

	g_main_loop_run(core->loop);
	return core;	
}











void
session_core_finalize(SessionCore* self, 
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
session_core_get_pipeline(SessionCore* self)
{
	return self->pipe;
}

WebRTCHub*
session_core_get_rtc_hub(SessionCore* self)
{
	return self->hub;
}


StreamConfig*
session_core_get_qoe(SessionCore* self)
{
	return self->qoe;
}


SignallingHub*
session_core_get_signalling_hub(SessionCore* core)
{
	return core->signalling;
}
#ifndef G_OS_WIN32
Display*
session_core_display_interface(SessionCore* self)
{
	return self->x_display;

}
#endif