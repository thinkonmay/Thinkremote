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
#include <agent-server.h>
#include <global-var.h>


#include <gst/gst.h>
#include <glib-2.0/glib.h>
#include <libsoup/soup.h>
#include <message-form.h>

#define GST_DEBUG               4
#define    THINKMAY_ACCOUNT_URL              "https://host.thinkmay.net/Account/Login"



static GOptionEntry entries[] = {
  {"token", 0, 0, G_OPTION_ARG_STRING, &TOKEN,
      "token register with worker manager", "TOKEN"},
  {"agentport", 0, 0, G_OPTION_ARG_STRING, &AGENT_PORT,
      "String ID of the peer to connect to", "ID"},
  {"coreport", 0, 0, G_OPTION_ARG_STRING, &SESSION_CORE_PORT,
      "Signalling server to connect to", "URL"},
  {"clusterip", 0, 0, G_OPTION_ARG_STRING, &CLUSTER_URL,
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
    default_var();
    GOptionContext *context;
    GError *error = NULL;

    context = g_option_context_new ("- thinkmay agent ");
    g_option_context_add_main_entries (context, entries, NULL);
    g_option_context_add_group (context, gst_init_get_option_group ());
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
    if(!strlen(CLUSTER_URL))
    {
        g_print("thinkmay cluster manager URL:\n[URL]: ");
        scanf("%s", CLUSTER_URL);
    }


    JsonObject* login = json_object_new();
    json_object_set_string_member(login,"UserName",USER);
    json_object_set_string_member(login,"Password",PASSWORD);
    gchar* login_body = get_string_from_json_object(login);


    SoupMessage* message = soup_message_new(SOUP_METHOD_POST,THINKMAY_ACCOUNT_URL);
    soup_message_set_request(message,"application/json",SOUP_MEMORY_COPY,login_body,strlen(login_body));
    soup_session_send_message(session,message);

    JsonParser* parser = json_parser_new();
    JsonObject* result_json = get_json_object_from_string(message->response_body->data,&error,parser);
    gchar* token_result = json_object_get_string_member(result_json,"token");
    g_object_unref(parser); 
    if(!token_result) {
        g_printerr("fail to login, retry\n");
        return;
    }
    else
    {
        g_print("login success\n");
    }

    memcpy(TOKEN,token_result,strlen(token_result));
    agent_new();
    return;
}

