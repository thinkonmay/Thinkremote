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





void
worker_log_output(gchar* text)
{

    if(!DEVELOPMENT_ENVIRONMENT && CLUSTER_URL)
    {
        const gchar* http_aliases[] = { "http", NULL };
        SoupSession* session = soup_session_new_with_options(
                SOUP_SESSION_SSL_STRICT, FALSE,
                SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
                SOUP_SESSION_HTTPS_ALIASES, http_aliases, NULL);

        // get log url from clusterip
        GString* url= g_string_new(CLUSTER_URL);
        g_string_append(url,"/worker/log");
        gchar* log_url = g_string_free(url,FALSE);
        SoupMessage* message = soup_message_new(SOUP_METHOD_POST,log_url);
        soup_message_headers_append(message->request_headers,"Authorization",DEVICE_TOKEN);

        // copy from buffer to soup message
        GString* string = g_string_new("\"");
        g_string_append(string,text);
        g_string_append(string,"\"");
        gchar* buffer = g_string_free(string,FALSE);

        soup_message_set_request(message,"application/json",SOUP_MEMORY_COPY,
            buffer,strlen(buffer));

        soup_session_send_async(session,message,NULL,NULL,NULL);    
    }
}                  