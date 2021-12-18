/**
 * @file agent-socket.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <agent-socket.h>
#include <agent-type.h>
#include <agent-session-initializer.h>
#include <agent-device.h>
#include <agent-server.h>

#include <logging.h>
#include <message-form.h>
#include <global-var.h>

#include <glib-object.h>
#include <json-glib/json-glib.h>
#include <libsoup/soup.h>


#define WRITE_TOKEN_TO_FILE TRUE

/// <summary>
/// contain information about websocket socket with host
/// </summary>
struct _Socket
{
    /// <summary>
    /// 
    /// </summary>
    SoupSession* session;
    
	/// <summary>
	/// 
	/// </summary>
	gchar cluster_url[50];
};








gboolean
send_message_to_cluster(AgentServer* object,
                     gchar* endpoint,
                     gchar* message)
{
    Socket* socket = agent_get_socket(object);
    GString* messsage_url = g_string_new(socket->cluster_url);
    g_string_append(messsage_url,endpoint);
    gchar* url = g_string_free(messsage_url,FALSE);

    SoupMessage* soupMessage = soup_message_new(SOUP_METHOD_POST,url);
    soup_message_headers_append(soupMessage->request_headers,"Authorization",DEVICE_TOKEN);

    if(message)
    {
        soup_message_set_request(soupMessage,"application/json",SOUP_MEMORY_COPY,
            message,strlen(message));
    }
    else
    {
        soup_message_set_request(soupMessage,"application/json",SOUP_MEMORY_COPY,"",0);
    }

    soup_session_send_message(socket->session,soupMessage);
}



gboolean
register_with_host(AgentServer* agent)
{
    GError* error = NULL;
    Socket* socket = agent_get_socket(agent);
    worker_log_output("Registering with host");

    gchar* package = get_registration_message();
    GString* register_url = g_string_new(socket->cluster_url);
    g_string_append(register_url,"/register");
    gchar* final_url = g_string_free(register_url,FALSE);

    SoupMessage* soupMessage = soup_message_new(SOUP_METHOD_POST,final_url);

    soup_message_headers_append(soupMessage->request_headers,"Authorization",TOKEN);

    soup_message_set_request(soupMessage,"application/json", SOUP_MEMORY_COPY,
        package,strlen(package));

    soup_session_send_message(socket->session,soupMessage);

    if(soupMessage->status_code != SOUP_STATUS_OK)
    {
        g_printerr("Fail to register device and get worker token\n");
        agent_finalize(agent);
        return;
    }
    else
    {
        JsonParser* parser = json_parser_new();
        JsonObject* result_json = get_json_object_from_string(soupMessage->response_body->data,&error,parser);
        gchar* token_result = json_object_get_string_member(result_json,"token");

        if(WRITE_TOKEN_TO_FILE)
        {
            GFile* file = g_file_new_for_path("./remote-token");
            g_file_delete(file,NULL,NULL);
            file = g_file_new_for_path("./remote-token");
            GFileOutputStream* stream = g_file_append_to(file,G_FILE_CREATE_REPLACE_DESTINATION,NULL,NULL);
            GOutputStream* output_Stream = (GOutputStream*)stream;
            g_output_stream_write_all(output_Stream,token_result,strlen(token_result),
                NULL,NULL,NULL);
        }

        if(token_result) 
        { 
            memcpy(DEVICE_TOKEN,token_result,strlen(token_result));
            g_print("Register successfully with cluster manager and got worker token\n");
            g_object_unref(parser); 
            return FALSE; 
        }
        else
        {
            g_print("receive package from cluster do dotn include worker token, aborting...\n");
            agent_finalize(agent);
            return FALSE; 
        }
    }
    


    return TRUE; 
}







Socket*
initialize_socket()
{
    Socket* socket = malloc(sizeof(Socket));
    memset(socket,0,sizeof(Socket)); 
    const gchar* http_aliases[] = { "http", NULL };

    GString* string = g_string_new("http://");
    g_string_append(string,CLUSTER_IP);
    g_string_append(string,":5000/agent");
    gchar* url = g_string_free(string,FALSE);

    memcpy( socket->cluster_url,url,strlen(url)); 
    socket->session = soup_session_new_with_options(
            SOUP_SESSION_SSL_STRICT, FALSE,
            SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
            SOUP_SESSION_HTTPS_ALIASES, http_aliases, NULL);

    return socket;
}