/**
 * @file main.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <glib.h>
#include <global-var.h>
#include <agent-server.h>


#include <gst/gst.h>
#include <glib-2.0/glib.h>
#include <libsoup/soup.h>
#include <message-form.h>

#include <development.h>
#include <stdio.h>

#define GST_DEBUG               4




static GOptionEntry entries[] = {
  {"token", 0, 0, G_OPTION_ARG_STRING, &TOKEN,
      "token register with worker manager", "TOKEN"},
  {"clusterurl", 0, 0, G_OPTION_ARG_STRING, &CLUSTER_URL,
      "Signalling server to connect to", "URL"},
  {"user", 0, 0, G_OPTION_ARG_STRING, &USER,
      "thinkmay manager username", "URL"},
  {"password", 0, 0, G_OPTION_ARG_STRING, &PASSWORD,
      "thinkmay manager password", "URL"},
  {NULL},
};


int
main(int argc, char* argv[])
{
    remote_application_init();
    GOptionContext *context;
    GError *error = NULL;

    context = g_option_context_new ("- thinkshare");
    g_option_context_add_main_entries (context, entries, NULL);
    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_printerr ("Error initializing: %s\n", error->message);
        return -1;
    }

    const gchar* http_aliases[] = { "https", NULL };
    SoupSession* session = soup_session_new_with_options(
            SOUP_SESSION_SSL_STRICT, FALSE,
            SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
            SOUP_SESSION_HTTPS_ALIASES, http_aliases, NULL);

    if(!strlen(USER))
    {
        g_print("Enter your thinkmay manager username:\n[USERNAME]: ");
        scanf("%s", USER);
    }
    if(!strlen(PASSWORD))
    {
        g_print("Enter your thinkmay manager password:\n[PASSWORD]: ");
        scanf("%s", PASSWORD);
    }
    if(!strlen(CLUSTER_NAME))
    {
        g_print("thinkmay cluster manager NAME:\n[NAME]: ");
        scanf("%s", CLUSTER_NAME);
    }






    JsonObject* login = json_object_new();
    json_object_set_string_member(login,"UserName",USER);
    json_object_set_string_member(login,"Password",PASSWORD);
    gchar* login_body = get_string_from_json_object(login);


    SoupMessage* message = soup_message_new(SOUP_METHOD_POST,ACCOUNT_URL);
    soup_message_set_request(message,"application/json",SOUP_MEMORY_COPY,login_body,strlen(login_body));
    soup_session_send_message(session,message);

    JsonParser* user_parser = json_parser_new();
    JsonObject* user_request_result = get_json_object_from_string(message->response_body->data,&error,user_parser);
    gchar* user_token = json_object_get_string_member(user_request_result,"token");
    memcpy(TOKEN,user_token,strlen(user_token));
    g_object_unref(user_parser); 
    if(!user_token) {
        g_printerr("fail to login, retry\n");
        return;
    }
    else
    {
        g_print("login success\n");
    }



    GString* string = g_string_new(CLUSTER_URL);
    g_string_append(string,"?ClusterName=");
    g_string_append(string,CLUSTER_NAME);
    gchar* cluster_token_url = g_string_free(string,FALSE);

    SoupMessage* cluster_message = soup_message_new(SOUP_METHOD_GET,cluster_token_url);
    soup_message_headers_append(cluster_message->request_headers, "Authorization",TOKEN);
    soup_session_send_message(session,cluster_message);

    JsonParser* cluster_parser = json_parser_new();
    JsonObject* cluster_request_result = get_json_object_from_string(cluster_message->response_body->data,&error,cluster_parser);
    gchar* cluster_token_received = json_object_get_string_member(cluster_request_result,"token");
    memcpy(CLUSTER_TOKEN,cluster_token_received,strlen(cluster_token_received));
    g_object_unref(cluster_parser); 




    SoupMessage* cluster_infor_message = soup_message_new(SOUP_METHOD_GET,CLUSTER_INFOR);
    soup_message_headers_append(cluster_infor_message->request_headers, "Authorization",cluster_token);
    soup_session_send_message(session,cluster_infor_message);

    JsonParser* cluster_infor_parser = json_parser_new();
    JsonObject* cluster_infor_result = get_json_object_from_string(cluster_infor_message->response_body->data,&error,cluster_infor_parser);

    gboolean self_host = json_object_get_boolean_member(cluster_infor_result,"selfHost");
    if(!self_host)
    {
        JsonObject* instance = json_object_get_object_member(cluster_infor_result,"instance");
        gchar* ipAddress = json_object_get_string_member(instance,"ipAdress");


        GString* cluster_url_string = g_string_new("http://");
        g_string_append(cluster_url_string,ipAddress);
        g_string_append(cluster_url_string,":5000");
        gchar* cluster_url_result = g_string_free(cluster_url_string,FALSE);

        memcpy(CLUSTER_URL, cluster_url_result,strlen(cluster_url_result));
    }
    else
    {
        g_print("thinkmay cluster manager URL:\n[URL]: ");
        scanf("%s", CLUSTER_URL);
    }


    g_object_unref(cluster_infor_parser); 
    agent_new(self_host);
    return;
}

