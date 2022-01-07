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
};


RemoteSession*
intialize_remote_session_service()
{
    RemoteSession* remote = malloc(sizeof(RemoteSession));
    return remote;
}


static void
handler_session_core_state_function(ChildProcess* proc,
                                    AgentServer* agent)
{
    RemoteSession* session = agent_get_remote_session(agent);
    send_message_to_cluster(agent,"/worker/session/end",NULL);
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
    g_string_append(core_script,CLUSTER_URL);
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