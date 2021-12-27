/**
 * @file token-validate.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <global-var.h>
#include <glib-2.0/glib.h>

#include <token-validate.h>



gboolean
validate_token(gchar* token)
{
    static gchar* http_aliases[] = { "http", NULL };
    SoupSession* session = soup_session_new_with_options(
            SOUP_SESSION_SSL_STRICT, FALSE,
            SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
            SOUP_SESSION_HTTPS_ALIASES, http_aliases, NULL);

    GString* token_url= g_string_new("http://");
    g_string_append(token_url, CLUSTER_IP);
    g_string_append(token_url,":5000/token/validate?token=");
    g_string_append(token_url, token);
    gchar* token_str = g_string_free(token_url,FALSE);

    SoupMessage* message = soup_message_new(SOUP_METHOD_POST,token_str);
    soup_message_set_request(message,"application/json",SOUP_MEMORY_COPY
        ,NULL,0);

    soup_session_send_message(session,message);

    if(message->status_code == SOUP_STATUS_OK)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}