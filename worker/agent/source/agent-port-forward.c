/**
 * @file agent-port-forward.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2022-01-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <agent-session-initializer.h>
#include <agent-socket.h>
#include <agent-type.h>
#include <agent-child-process.h>
#include <agent-port-forward.h>

#include <message-form.h>
#include <general-constant.h>
#include <global-var.h>
#include <logging.h>

#include <gmodule.h>
#include <stdio.h>

#define BUFFER_SIZE 10000


static void
handle_portforward_disconnected(ChildProcess* proc,
                                AgentServer* agent)
{
    start_portforward(agent);
}


static void
handle_portfoward_error(GBytes* buffer,
                        AgentServer* agent,
                        gpointer data)
{
    gchar* message = g_bytes_get_data(buffer, NULL);
}

static void
handle_portforward_output(GBytes* buffer,
                           AgentServer* agent,
                           gpointer data)
{
    gchar* message = g_bytes_get_data(buffer, NULL);
}



void
start_portforward(AgentServer* agent)
{
    // return false if session core is running before the initialization
    GString* core_script = g_string_new(PORT_FORWARD_BINARY);
    g_string_append(core_script," ");
    g_string_append(core_script,CLUSTER_TOKEN);
    g_string_append(core_script," ");
    g_string_append(core_script,AGENT_PORT);
    g_string_append(core_script," ");
    g_string_append(core_script,SESSION_CORE_PORT);
    gchar* process_path = g_string_free(core_script,FALSE);

    create_new_child_process(process_path,
        (ChildStdErrHandle)handle_portfoward_error,
        (ChildStdOutHandle)handle_portforward_output,
        (ChildStateHandle)handle_portforward_disconnected, agent,NULL);
}