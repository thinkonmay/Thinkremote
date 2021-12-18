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
#include <glib.h>
#include <logging.h>
#include <gio/gio.h>
#include <string.h>

#include <libsoup/soup.h>
#include <global-var.h>
#include <development.h>
#include <agent-device.h>



void 
on_log_finished(GObject *object,
                GAsyncResult *res,
                gpointer user_data)
{
    GError* error = NULL;
    SoupMessage* message = (SoupMessage*) user_data;
    GInputStream* stream = soup_session_send_finish(object,res,&error);

    if(error)
        g_print("%s\n",error->message);
    
    if(message->status_code == SOUP_STATUS_BAD_REQUEST)
        g_print("log success with result code: %d:\n%s\n",message->status_code,message->response_body->data);
    
}




void
worker_log_output(gchar* text)
{

    if(!DEVELOPMENT_ENVIRONMENT)
    {
        const gchar* http_aliases[] = { "http", NULL };
        SoupSession* session = soup_session_new_with_options(
                SOUP_SESSION_SSL_STRICT, FALSE,
                SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
                SOUP_SESSION_HTTPS_ALIASES, http_aliases, NULL);

        // get log url from clusterip
        GString* url= g_string_new("http://");
        g_string_append(url,CLUSTER_IP);
        g_string_append(url,":5000/log?ip=");
        g_string_append(url,get_local_ip());
        gchar* log_url = g_string_free(url,FALSE);
        SoupMessage* message = soup_message_new(SOUP_METHOD_POST,log_url);



        GString* string =  g_string_new("\"");
        g_string_append(string,text);
        g_string_append(string,"\"");
        gchar* body = g_string_free(string,FALSE);


        // copy from buffer to soup message
        soup_message_set_request(message,"application/json",SOUP_MEMORY_COPY,
            body,strlen(body));

        soup_session_send_async(session,message,NULL,(GAsyncReadyCallback)on_log_finished,message);    
    }
}                  