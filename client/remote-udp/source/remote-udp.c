/**
 * @file remote-udp.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <remote-udp-pipeline.h>
#include <remote-udp.h>
#include <overlay-gui.h>
#include <remote-udp-type.h>

#include <constant.h>
#include <remote-config.h>
#include <global-var.h>
#include <key-convert.h>


#include <glib.h>
#include <gst/base/gstbasesink.h>
#include <json-handler.h>
#include <libsoup/soup.h>
#include <overlay-gui.h>



struct _RemoteUdp
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
	GMainLoop* loop;

	/**
	 * @brief 
	 * 
	 */
	StreamConfig* qoe;

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
remote_app_setup_session(RemoteUdp* self, 
						 gchar* remote_token)
{    
	if(!DEVELOPMENT_ENVIRONMENT)
		setup_pipeline_startpoint(self->pipe, 6001, 6002);

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
		remote_app_finalize(self,NULL);

	GError* error = NULL;
	JsonParser* parser = json_parser_new();
	JsonObject* json_infor = get_json_object_from_string(infor_message->response_body->data,error,parser);


	qoe_setup(self->qoe,
				json_object_get_int_member(json_infor,"audiocodec"),
				json_object_get_int_member(json_infor,"videocodec"),
				OPUS_ENC,CODEC_H265,ULTRA_HIGH_CONST);

	g_object_unref(parser);
}


static Shortcut*
get_default_shortcut(gpointer data)
{
    RemoteUdp* udp = (RemoteUdp*)data;
    Shortcut* shortcuts = malloc(sizeof(Shortcut)*10);
    memset(shortcuts,0,sizeof(Shortcut)*10);

    (shortcuts + 0)->active = TRUE;
    (shortcuts + 0)->data = udp;
    (shortcuts + 0)->function = remote_app_reset;
    (shortcuts + 0)->opcode = RELOAD_STREAM;

    (shortcuts + 0)->key_list[0] = W_KEY;
    (shortcuts + 0)->key_list[1] = VK_SHIFT;
    (shortcuts + 0)->key_list[2] = VK_CONTROL;
    (shortcuts + 0)->key_list[3] = VK_MENU;

    return shortcuts;
}

void
send_hid_message(gchar* message,
				 gpointer data)
{

}


RemoteUdp*
remote_app_initialize(gchar* remote_token)
{
	if(DEVELOPMENT_ENVIRONMENT)
		g_print("Starting remote app in development environment \n");
	else
		g_print("Initializing remote app, please wait ...\n");


	RemoteUdp* app= 		malloc(sizeof(RemoteUdp));
	app->loop =				g_main_loop_new(NULL, FALSE);

	Shortcut* shortcuts = 	get_default_shortcut(app);
	app->gui =				init_remote_app_gui(app,shortcuts,send_hid_message);
	free(shortcuts);

	app->qoe =				qoe_initialize();
	app->pipe =				pipeline_initialize();
	 
	remote_app_setup_session(app, remote_token);

	setup_pipeline(app);
	g_main_loop_run(app->loop);
	return app;	
}




















void
remote_app_reset(RemoteUdp* self)
{
	setup_pipeline(self);
}

void
remote_app_finalize(RemoteUdp* self, 
					  GError* error)
{
	if(error)
		g_print(error->message);

	gui_terminate(self->gui);
	g_main_loop_quit(self->loop);
}









Pipeline*
remote_app_get_pipeline(RemoteUdp* self)
{
	return self->pipe;
}


StreamConfig*
remote_app_get_qoe(RemoteUdp* self)
{
	return self->qoe;
}

GUI*
remote_app_get_gui(RemoteUdp* core)
{
	return core->gui;
}

