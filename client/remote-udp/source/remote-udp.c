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
#include <remote-udp-type.h>

#include <constant.h>
#include <remote-config.h>
#include <global-var.h>
#include <key-convert.h>
#include <shortcut.h>


#include <glib.h>
#include <gst/base/gstbasesink.h>
#include <json-handler.h>
#include <libsoup/soup.h>
#include <device.h>
#include <stdio.h>

#ifdef G_OS_WIN32
#include <overlay-gui.h>
#else
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#endif



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

#ifdef G_OS_WIN32
	/**
	 * @brief 
	 * 
	 */
	GUI* gui;
#endif
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

	gchar* ip = malloc(20);
	g_print("Enter target ip:");
	scanf("%s",ip);
	
	JsonObject* json = json_object_new();
	json_object_set_string_member(json,"AUDIO_PORT","6001");
	json_object_set_string_member(json,"AUDIO_HOST",get_local_ip());
	json_object_set_string_member(json,"VIDEO_PORT","6002");
	json_object_set_string_member(json,"VIDEO_HOST",get_local_ip());

#ifdef G_OS_WIN32
	json_object_set_int_member(json,"Device",WINDOW_APP);
	json_object_set_int_member(json,"Engine",GSTREAMER);
#else
	json_object_set_int_member(json,"Device",LINUX_APP);
	json_object_set_int_member(json,"Engine",GSTREAMER);
#endif

	gchar* body = get_string_from_json_object(json);

	const char* https_aliases[] = { "http", NULL };
	SoupSession* https_session = soup_session_new_with_options(
			SOUP_SESSION_SSL_STRICT, FALSE,
			SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
			SOUP_SESSION_HTTPS_ALIASES, https_aliases, NULL);
		


	GString* string = g_string_new("http://");
	g_string_append(string,ip);
	g_string_append(string,":3567/Initialize");
	
	SoupMessage* infor_message = soup_message_new(SOUP_METHOD_POST,g_string_free(string,FALSE));
	soup_message_set_request(infor_message,"application/json",SOUP_MEMORY_COPY,body,strlen(body));
	soup_session_send_message(https_session,infor_message);

	setup_pipeline(self);
}


static Shortcut*
get_default_shortcut(gpointer data)
{
	Shortcut* shortcuts = shortcut_list_initialize(10);

	gint key_list[10] = {0};
#ifdef G_OS_WIN32
    key_list[0] = W_KEY;
    key_list[1] = VK_SHIFT;
    key_list[2] = VK_CONTROL;
    key_list[3] = VK_MENU;
#endif

	add_new_shortcut_to_list(shortcuts,key_list,
			RELOAD_STREAM,remote_app_reset,data);

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
#ifdef G_OS_WIN32
	app->gui =				init_remote_app_gui(app,shortcuts,send_hid_message);
#endif
	shortcut_list_free(shortcuts);

	app->qoe =				qoe_initialize();
	app->pipe =				pipeline_initialize();
	 
	remote_app_setup_session(app, remote_token);
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

#ifdef G_OS_WIN32
GUI*
remote_app_get_gui(RemoteUdp* core)
{
	return core->gui;
}
#endif

