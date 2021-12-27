/**
 * @file agent-session-initializer.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <agent-session-initializer.h>
#include <agent-socket.h>
#include <agent-type.h>
#include <agent-child-process.h>

#include <message-form.h>
#include <general-constant.h>
#include <global-var.h>
#include <logging.h>

#include <gmodule.h>
#include <stdio.h>

#define BUFFER_SIZE 10000


struct _RemoteSession
{
    ChildProcess* process;

    gchar session_core_url[50];

    SoupSession* session;
};


RemoteSession*
intialize_remote_session_service()
{
    RemoteSession* remote = malloc(sizeof(RemoteSession));
    GString* base_url = g_string_new("http://localhost:");
    g_string_append(base_url,SESSION_CORE_PORT);
    g_string_append(base_url,"/agent/message");
    gchar* url = g_string_free(base_url,FALSE);

    const gchar* http_aliases[] = { "http", NULL };
    remote->session = soup_session_new_with_options(
            SOUP_SESSION_SSL_STRICT, FALSE,
            SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
            SOUP_SESSION_HTTPS_ALIASES, http_aliases, NULL);

    remote->process = NULL;
    memset(remote->session_core_url,0,50);
    memcpy(remote->session_core_url,url,strlen(url));
    return remote;
}


static void
handler_session_core_state_function(ChildProcess* proc,
                                    AgentServer* agent)
{
    RemoteSession* session = agent_get_remote_session(agent);
    send_message_to_cluster(agent,"/core/end",NULL);
}


static void
handle_session_core_error(GBytes* buffer,
    AgentServer* agent,
    gpointer data)
{
    gchar* message = g_bytes_get_data(buffer, NULL);
}

static void
handle_session_core_output(GBytes* buffer,
    AgentServer* agent,
    gpointer data)
{
    gchar* message = g_bytes_get_data(buffer, NULL);
}


gboolean
session_terminate(AgentServer* agent)
{
    RemoteSession* session = agent_get_remote_session(agent);

    // return true if session core is not running before termination
    if(!session->process)
        return TRUE;

    childprocess_force_exit(session->process);
    clean_childprocess(session->process);
    session->process = NULL;

    // return true 
    return TRUE;
}

gboolean
session_initialize(AgentServer* agent)
{
    RemoteSession* session = agent_get_remote_session(agent);

    // return false if session core is running before the initialization
    GString* core_script = g_string_new(SESSION_CORE_BINARY);
    g_string_append(core_script," --token=");
    g_string_append(core_script,DEVICE_TOKEN);
    g_string_append(core_script," --clusterip=");
    g_string_append(core_script,CLUSTER_IP);
    gchar* process_path = g_string_free(core_script,FALSE);

    session->process =
    create_new_child_process(process_path,
        (ChildStdErrHandle)handle_session_core_error,
        (ChildStdOutHandle)handle_session_core_output,
        (ChildStateHandle)handler_session_core_state_function, agent,NULL);
    
    if(!session->process)
        return FALSE;
    else    
        return TRUE;
}

gboolean
send_message_to_core(AgentServer* agent, 
                     gchar* buffer)
{
    RemoteSession* session = agent_get_remote_session(agent);

    SoupMessage* message = soup_message_new(SOUP_METHOD_POST,session->session_core_url);
    soup_message_headers_append(message->request_headers,"Authorization",DEVICE_TOKEN);

    soup_message_set_request(message,"application/json",SOUP_MEMORY_COPY,
        buffer,strlen(buffer)); 

    soup_session_send_async(session->session,message,NULL,NULL,NULL);
}