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
#include <json-handler.h>

#include <constant.h>
#include <logging.h>
#include <stdio.h>





/**
 * @brief 
 * 
 */
const gchar* cluster_url; 


/**
 * @brief 
 * 
 */
const gchar* cluster_token; 

/**
 * @brief 
 * cluster owner username 
 */
const gchar* user; 

/**
 * @brief 
 * cluster owner password
 */
const gchar* password; 


/**
 * @brief 
 * 
 */
const gchar* cluster_name ; 

/**
 * @brief 
 * user token to communication with other module
 */
const gchar* token;

/**
 * @brief 
 * user token to communication with other module
 */
const gchar* environment;

/**
 * @brief 
 * user token to communication with other module
 */
const gchar* input_capture;

static GOptionEntry entries[] = {
  {"token", 0, 0, G_OPTION_ARG_STRING, &token,
      "token register with worker manager", "TOKEN"},
  {"cluster", 0, 0, G_OPTION_ARG_STRING, &cluster_name,
      "Signalling server to connect to", "NAME"},
  {"user", 0, 0, G_OPTION_ARG_STRING, &user,
      "thinkmay manager username", "USERNAME"},
  {"password", 0, 0, G_OPTION_ARG_STRING, &password,
      "thinkmay manager password", "PASSWORD"},
  {"environment", 0, 0, G_OPTION_ARG_STRING, &environment,
      "environment (development,localhost,production(default))", "ENV"},
  {NULL},
};

#ifdef G_OS_WIN32
void clear() {
    COORD topLeft  = { 0, 0 };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;

    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(
        console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    FillConsoleOutputAttribute(
        console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
        screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    SetConsoleCursorPosition(console, topLeft);
}
#endif

int
main(int argc, char* argv[])
{
    user =              malloc(100);
    cluster_url =       malloc(100);
    environment =       malloc(100);
    password =          malloc(100);
    cluster_name =      malloc(100);

    cluster_token =     malloc(500);
    token =             malloc(500);

    memset(user,0,100);
    memset(cluster_url,0,100);
    memset(environment,0,100);
    memset(password,0,100);
    memset(cluster_name,0,100);
    
    memset(token,0,500);
    memset(cluster_token,0,500);



    GOptionContext *context;
    GError *error = NULL;

    context = g_option_context_new ("- thinkshare");
    g_option_context_add_main_entries (context, entries, NULL);
    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_printerr ("Error initializing: %s\n", error->message);
        return -1;
    }

    if(
        !g_strcmp0(environment,"development") ||
        !g_strcmp0(environment,"localhost"))
    {
        thinkremote_application_init(environment, NULL, NULL, NULL);
        agent_new(NULL);
        return;
    }

    const gchar* http_aliases[] = { "https", NULL };
    SoupSession* session = soup_session_new_with_options(
            SOUP_SESSION_SSL_STRICT, FALSE,
            SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
            SOUP_SESSION_HTTPS_ALIASES, http_aliases, NULL);

    if(!strlen(user))
    {
        g_print("Enter your thinkmay manager username:\n[USERNAME]: ");
        scanf("%s", user);
    }
    if(!strlen(password))
    {
        g_print("Enter your thinkmay manager password:\n[PASSWORD]: ");
        scanf("%s", password);
    }
    if(!strlen(cluster_name))
    {
        g_print("thinkmay cluster manager NAME:\n[NAME]: ");
        scanf("%s", cluster_name);
    }
#ifdef G_OS_WIN32
    clear();    
#endif

    {
        JsonObject* login = json_object_new();
        json_object_set_string_member(login,"UserName",user);
        json_object_set_string_member(login,"Password",password);
        gchar* login_body = get_string_from_json_object(login);


        SoupMessage* message = soup_message_new(SOUP_METHOD_POST,ACCOUNT_URL);
        soup_message_set_request(message,"application/json",SOUP_MEMORY_COPY,login_body,strlen(login_body));
        soup_session_send_message(session,message);

        JsonParser* parser = json_parser_new();
        JsonObject* result = get_json_object_from_string(message->response_body->data,&error,parser);
        gchar* user_token = json_object_get_string_member(result,"token");

        g_assert_nonnull(user_token);
        memcpy(token,user_token,strlen(user_token));
        g_object_unref(parser); 
    }


    {
        GString* string = g_string_new(CLUSTER_TOKEN_URL);
        g_string_append(string,"?ClusterName=");
        g_string_append(string,cluster_name);
        gchar* cluster_token_url = g_string_free(string,FALSE);

        SoupMessage* message = soup_message_new(SOUP_METHOD_GET,cluster_token_url);
        soup_message_headers_append(message->request_headers, "Authorization",token);
        soup_session_send_message(session,message);

        if(message->status_code == 401)
            g_printerr("User have evelvated to cluster manager yet");

        JsonParser* parser = json_parser_new();
        JsonObject* result = get_json_object_from_string(message->response_body->data,&error,parser);
        gchar* cluster_token_received = json_object_get_string_member(result,"token");

        g_assert_nonnull(cluster_token_received);
        memcpy(cluster_token,cluster_token_received,strlen(cluster_token_received));
        g_object_unref(parser); 
    }


    {
        SoupMessage* message = soup_message_new(SOUP_METHOD_GET,INSTANCE_INFOR_URL);
        soup_message_headers_append(message->request_headers, "Authorization",cluster_token);
        soup_session_send_message(session,message);

        JsonParser* parser = json_parser_new();
        JsonObject* instance = get_json_object_from_string(message->response_body->data,&error,parser);
        gchar* ipAddress = json_object_get_string_member(instance,"ipAdress");


        GString* cluster_url_string = g_string_new("http://");
        g_string_append(cluster_url_string,ipAddress);
        g_string_append(cluster_url_string,":5000");
        gchar* cluster_url_result = g_string_free(cluster_url_string,FALSE);

        g_assert_nonnull(cluster_url_result);
        memcpy(cluster_url, cluster_url_result,strlen(cluster_url_result));
        g_object_unref(parser);
    }

    thinkremote_application_init(environment,
                                cluster_url,
                                cluster_token,
                                NULL);

    agent_new(token);
    return;
}

