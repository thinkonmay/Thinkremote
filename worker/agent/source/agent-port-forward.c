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
#include <development.h>

#include <gmodule.h>
#include <stdio.h>

#define BUFFER_SIZE 10000

#ifdef G_OS_WIN32
#include <Windows.h>
#endif

struct _PortForward
{
    ChildProcess* process;

    SoupSession* host_session;

    gchar agent_instance_port[20];

    gchar core_instance_port[20];

    gchar port_release_url[50];
};



#define PORT_RELEASE_URL "https://development.thinkmay.net/Port/Release?InstancePort="


PortForward*
init_portforward_service()
{
    PortForward* port = malloc(sizeof(PortForward));
    memset(port,0,sizeof(PortForward));


    const gchar* https_aliases[] = { "https", NULL };
    port->host_session = soup_session_new_with_options(
            SOUP_SESSION_SSL_STRICT, FALSE,
            SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
            SOUP_SESSION_HTTPS_ALIASES, https_aliases, NULL);
    return port;
}




static void
handle_portforward_disconnected(ChildProcess* proc,
                                AgentServer* agent)
{
    PortForward* port = agent_get_portforward(agent);

    GString* agent_request_url_string = g_string_new(PORT_RELEASE_URL);
    GString* core_request_url_string  = g_string_new(PORT_RELEASE_URL);

    g_string_append(agent_request_url_string,port->agent_instance_port);
    g_string_append(core_request_url_string,port->core_instance_port);

    gchar* agent_request_url = g_string_free(agent_request_url_string,FALSE);
    gchar* core_request_url = g_string_free(core_request_url_string,FALSE);

    SoupMessage* agent_request = soup_message_new(SOUP_METHOD_GET,agent_request_url);
    soup_message_headers_append(agent_request->request_headers,"Authorization",CLUSTER_TOKEN);
    soup_message_set_request(agent_request,"application/json", SOUP_MEMORY_COPY, "",0);

    SoupMessage* core_request  = soup_message_new(SOUP_METHOD_GET,core_request_url);
    soup_message_headers_append(core_request->request_headers,"Authorization",CLUSTER_TOKEN);
    soup_message_set_request(agent_request,"application/json", SOUP_MEMORY_COPY, "",0);

    soup_session_send_message(port->host_session,core_request);
    soup_session_send_message(port->host_session,agent_request);

    memset(port->agent_instance_port,0,20);  
    memset(port->core_instance_port,0,20);  

    while (!start_portforward(agent))
    {
#ifdef G_OS_WIN32
        Sleep(10000);
#else
        sleep(10000);
#endif
    }
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



gpointer
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

    ChildProcess* process = create_new_child_process(process_path,
        (ChildStdErrHandle)handle_portfoward_error,
        (ChildStdOutHandle)handle_portforward_output,
        (ChildStateHandle)handle_portforward_disconnected, agent,NULL);
    return process;
}





void
describe_portforward(PortForward* port_forward, 
                     gchar* agent_instance_port,
                     gchar* core_instance_port)
{
    memcpy(port_forward->agent_instance_port, agent_instance_port, strlen(agent_instance_port)); 
    memcpy(port_forward->core_instance_port, core_instance_port,strlen(core_instance_port)); 
}