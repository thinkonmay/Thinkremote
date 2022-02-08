/**
 * @file remote-webrtc.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <remote-webrtc-signalling.h>
#include <remote-webrtc-remote-config.h>
#include <remote-webrtc-pipeline.h>
#include <remote-webrtc-data-channel.h>
#include <remote-webrtc.h>
#include <remote-webrtc-gui.h>
#include <remote-webrtc-type.h>
#include <remote-webrtc-input.h>

#include <module-code.h>
#include <development.h>


#include <glib.h>
#include <gst/base/gstbasesink.h>
#include <message-form.h>
#include <module-code.h>
#include <libsoup/soup.h>


struct _RemoteApp
{
	/**
	 * @brief 
	 * 
	 */
	Pipeline* pipe;

	/**
	 * @brief 
	 * 
	 */
	WebRTCHub* hub;

	/**
	 * @brief 
	 * 
	 */
	GMainLoop* loop;

	/**
	 * @brief 
	 * 
	 */
	SignallingHub* signalling;

	/**
	 * @brief 
	 * 
	 */
	InputHandler* handler;

	/**
	 * @brief 
	 * 
	 */
	QoE* qoe;

	/**
	 * @brief 
	 * 
	 */
	GUI* gui;
};


/**
 * @brief 
 * setup session 
 * @param self 
 * @param session_id 
 * @param signalling_url 
 * @param turn 
 * @param audio_codec 
 * @param video_codec 
 */
static void
remote_app_setup_session(RemoteApp* self, 
						 gchar* remote_token)
{    
	if(!DEVELOPMENT_ENVIRONMENT)
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


		SoupMessage* infor_message = soup_message_new(SOUP_METHOD_GET,infor_str);
		soup_session_send_message(https_session,infor_message);


		if(infor_message->status_code == SOUP_STATUS_OK)
		{
			GError* error = NULL;
			JsonParser* parser = json_parser_new();
			JsonObject* json_infor = get_json_object_from_string(infor_message->response_body->data,error,parser);


			signalling_hub_setup(self->signalling,
				json_object_get_string_member(json_infor,"turn"),
				json_object_get_string_member(json_infor,"signallingurl"),
				json_object_get_array_member(json_infor,"stuns"),
				remote_token);

			qoe_setup(self->qoe,
						json_object_get_int_member(json_infor,"audiocodec"),
						json_object_get_int_member(json_infor,"videocodec"));

			g_object_unref(parser);
		}
		else 
		{
			GError* error = malloc(sizeof(GError));
			g_printerr("response code: %d\n",infor_message->status_code);
			error->message = "fail to get session information";
			remote_app_finalize(self,error);
			return;
		}
	}
	else
	{
		signalling_hub_setup(self->signalling,
#ifdef DEFAULT_TURN
			DEFAULT_TURN,
#else
			" ",
#endif
			DEVELOPMENT_SIGNALLING_URL,
			NULL,
			remote_token);

		qoe_setup(self->qoe,
				OPUS_ENC,
				CODEC_H265);
	}
}




RemoteApp*
remote_app_initialize(gchar* remote_token)
{
	if(DEVELOPMENT_ENVIRONMENT)
		g_print("Starting remote app in development environment \n");
	else
		g_print("Initializing remote app, please wait ...\n");


	RemoteApp* app= 		malloc(sizeof(RemoteApp));
	app->loop =				g_main_loop_new(NULL, FALSE);
	app->handler =			init_input_capture_system(app);
	app->gui =				init_remote_app_gui(app);
	app->hub =				webrtchub_initialize();
	app->signalling =		signalling_hub_initialize(app);

	app->qoe =				qoe_initialize();
	app->pipe =				pipeline_initialize(app);
	 
	remote_app_setup_session(app, remote_token);
	signalling_connect(app);

	g_main_loop_run(app->loop);
	return app;	
}




















void
remote_app_reset(RemoteApp* self)
{
	stop_to_ping(self->hub);
	signalling_close(self->signalling);
	signalling_connect(self);
}

void
remote_app_finalize(RemoteApp* self, 
					  GError* error)
{
	if(error)
		g_print(error->message);

	gui_terminate(self->gui);
	signalling_close(self->signalling);
	g_main_loop_quit(self->loop);
}









Pipeline*
remote_app_get_pipeline(RemoteApp* self)
{
	return self->pipe;
}

WebRTCHub*
remote_app_get_rtc_hub(RemoteApp* self)
{
	return self->hub;
}


QoE*
remote_app_get_qoe(RemoteApp* self)
{
	return self->qoe;
}

GUI*
remote_app_get_gui(RemoteApp* core)
{
	return core->gui;
}

SignallingHub*
remote_app_get_signalling_hub(RemoteApp* core)
{
	return core->signalling;
}

InputHandler*
remote_app_get_hid_handler(RemoteApp* app)
{
	return app->handler;
}