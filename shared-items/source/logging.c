/**
 * @file logging.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <logging.h>
#include <string.h>

#include <libsoup/soup.h>
#include <global-var.h>
#include <constant.h>

#include <glib.h>
#include <json-handler.h>
#include <json-glib/json-glib.h>


#define IP "https://api.ipify.org?format=string"

static gchar source[200] = {0};

void 
ip_message_callback(GObject *source_object,
				    GAsyncResult *res,
				    gpointer user_data)
{
    gchar ip_address[100] = {0}; 
    GError* error = NULL;
    GInputStream* result = soup_session_send_finish(source_object,res,&error);

    g_input_stream_read_all(result,ip_address,100,NULL,NULL,NULL);

    JsonObject* object = json_object_new();
    json_object_set_string_member(object,"ClusterURL",CLUSTER_URL);
    json_object_set_string_member(object,"WorkerIP",ip_address);
    gchar* buffer = get_string_from_json_object(object);
    memcpy(source,buffer,strlen(buffer));

    g_input_stream_close(result,NULL,NULL);
}

void
worker_log_output(gchar* text)
{
    static gboolean initialized = FALSE;
    static SoupSession* session; 

    if(!initialized)
    {
        initialized = TRUE;
        const gchar* http_aliases[] = { "https", NULL };
        session = soup_session_new_with_options(
            SOUP_SESSION_SSL_STRICT, FALSE,
            SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
            SOUP_SESSION_HTTPS_ALIASES, http_aliases, NULL);

        SoupMessage* message = soup_message_new(SOUP_METHOD_GET,IP);
        soup_session_send_async(session,message,NULL,
            (GAsyncReadyCallback)ip_message_callback,NULL);    
    }

    if(DEVELOPMENT_ENVIRONMENT)
        g_print("%s\n",text);

    GTimeVal time;
    g_get_current_time(&time);
    gchar* time_string = g_time_val_to_iso8601(&time);
    JsonObject* object = json_object_new();
    json_object_set_string_member(object,"Log",text);
    json_object_set_string_member(object,"Source",source);
    json_object_set_string_member(object,"Type","Direct worker log");
    json_object_set_string_member(object,"timestamp",time_string);
    gchar* buffer = get_string_from_json_object(object);

    SoupMessage* message = soup_message_new(SOUP_METHOD_POST,LOG_URL);
    soup_message_set_request(message,"application/json",SOUP_MEMORY_COPY, buffer,strlen(buffer));
    soup_session_send_async(session,message,NULL,NULL,NULL);    
}                  