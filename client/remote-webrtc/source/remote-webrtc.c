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
#include <remote-webrtc-pipeline.h>
#include <remote-webrtc-data-channel.h>
#include <remote-webrtc.h>
#include <remote-webrtc-type.h>

#include <constant.h>
#include <environment.h>
#include <remote-config.h>
#include <global-var.h>
#include <overlay-gui.h>
#include <capture-key.h>
#include <key-convert.h>
#include <shortcut.h>


#include <glib.h>
#include <gst/base/gstbasesink.h>
#include <json-handler.h>
#include <libsoup/soup.h>
#include <stdio.h>


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
	GUI* gui;
};



void
remote_development_setup(RemoteApp* self)
{
	gchar* signalling = DEVELOPMENT_SIGNALLING_URL;
#ifdef DEFAULT_TURN
	gchar* turn =	DEFAULT_TURN;
#else
	gchar* turn =	" ";
#endif

	JsonArray* array = json_array_new();
	json_array_add_string_element(array,DEFAULT_STUN);

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


	signalling_hub_setup(self->signalling,
			turn,
			signalling,
			array,
			DEFAULT_CLIENT_TOKEN);
}



/**
 * @brief 
 * 
 * @param self 
 * @param remote_token 
 */
static void
remote_app_setup_session(RemoteApp* self, 
						 gchar* remote_token)
{    
	if(DEVELOPMENT_ENVIRONMENT)
	{
		remote_development_setup(self);
		return;
	}

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


	if(infor_message->status_code != SOUP_STATUS_OK)
	{
		g_printerr("fail to get session information: response code %d\n",infor_message->status_code);
		remote_app_finalize(self,NULL);
	}

	JsonParser* parser = json_parser_new();
	JsonObject* json_infor = get_json_object_from_string(infor_message->response_body->data,NULL,parser);

	signalling_hub_setup(self->signalling,
		json_object_get_string_member(json_infor,"turn"),
		json_object_get_string_member(json_infor,"signallingurl"),
		json_object_get_array_member(json_infor,"stuns"),
		remote_token);

	g_object_unref(parser);
}


static Shortcut*
get_default_shortcut(gpointer data)
{
    RemoteApp* app = (RemoteApp*)data;
	Shortcut* shortcuts = shortcut_list_initialize(10);

	gint key_list_reset[10] = {0};
	gint key_list_finalize[10] = {0};

    key_list_reset[0] = W_KEY;
    key_list_reset[1] = VK_SHIFT;
    key_list_reset[2] = VK_CONTROL;
    key_list_reset[3] = VK_MENU;


    key_list_finalize[0] = E_KEY;
    key_list_finalize[1] = VK_SHIFT;
    key_list_finalize[2] = VK_CONTROL;
    key_list_finalize[3] = VK_MENU;

	add_new_shortcut_to_list(shortcuts,key_list_finalize,EXIT,remote_app_finalize,app);
	add_new_shortcut_to_list(shortcuts,key_list_reset,RELOAD_STREAM,remote_app_reset,app);

    return shortcuts;
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

	Shortcut* shortcuts = 	get_default_shortcut(app);
	app->gui =				init_remote_app_gui(app,shortcuts,hid_data_channel_send);
	shortcut_list_free(shortcuts);

	app->hub =				webrtchub_initialize();
	app->signalling =		signalling_hub_initialize(app);
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