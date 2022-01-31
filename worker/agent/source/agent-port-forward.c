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
};

#define PORT_FILE  "./instancePort"
gchar* 
portforward_get_agent_instance_port(PortForward *port)
{
    return port->agent_instance_port;
}

#define PORT_RELEASE_URL "https://host.thinkmay.net/Port/Release?InstancePort="
#define PORT_OBTAIN_URL  "https://host.thinkmay.net/Port/Request"

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





    gint agent_instance_port;
    GError* agent_err = NULL;
    GError* error = NULL;
    gchar* buffer;
    gsize file_size;

    g_file_get_contents(PORT_FILE,&buffer,&file_size,&error);
    if(file_size > 0)
    {
        agent_instance_port = atoi(buffer);
    }
    else
    {
        SoupMessage* agent_request = soup_message_new(SOUP_METHOD_GET,PORT_OBTAIN_URL);
        soup_message_headers_append(agent_request->request_headers,"Authorization",CLUSTER_TOKEN);
        soup_message_set_request(agent_request,"application/json", SOUP_MEMORY_COPY, "",0);


        soup_session_send_message(port->host_session,agent_request);

        JsonParser* agent_parser = json_parser_new();
        JsonObject* agent_object = get_json_object_from_string(agent_request->response_body->data,&agent_err,agent_parser);
        agent_instance_port = json_object_get_int_member(agent_object,"instancePort");
    }
    



    itoa(agent_instance_port,port->agent_instance_port,10);
    memcpy(AGENT_PORT,port->agent_instance_port,10);






    return port;
}




static void
handle_portforward_disconnected(ChildProcess* proc,
                                AgentServer* agent)
{
    PortForward* port = agent_get_portforward(agent);
    memset(port->agent_instance_port,0,20);  
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


PortForward*
start_portforward(AgentServer* agent)
{
    PortForward* port = agent_get_portforward(agent);

#ifdef G_OS_WIN32
    SetEnvironmentVariable("port", TEXT(AGENT_PORT));
    SetEnvironmentVariable("clustertoken", TEXT(CLUSTER_TOKEN));
#endif

    // return false if session core is running before the initialization
    port->process = create_new_child_process(PORT_FORWARD_BINARY,
        (ChildStdErrHandle)handle_portfoward_error,
        (ChildStdOutHandle)handle_portforward_output,
        (ChildStateHandle)handle_portforward_disconnected, agent,NULL);

    if(port->process)
    {
        GFile* file = g_file_new_for_path(PORT_FILE);
        g_file_delete(file,NULL,NULL);
        file = g_file_new_for_path(PORT_FILE);
        GFileOutputStream* stream = g_file_append_to(file,G_FILE_CREATE_REPLACE_DESTINATION,NULL,NULL);
        GOutputStream* output_Stream = (GOutputStream*)stream;
        g_output_stream_write_all(output_Stream,port->agent_instance_port,strlen(port->agent_instance_port), NULL,NULL,NULL);
    }
    
    return port->process ? port : NULL;
}




