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

gboolean
SetPermanentEnvironmentVariable(LPCTSTR value, 
                                LPCTSTR data)
{
    HKEY hKey;
    LPCTSTR keyPath = TEXT("Environment");
    LSTATUS lOpenStatus = RegOpenKeyEx(HKEY_CURRENT_USER, keyPath, 0, KEY_ALL_ACCESS, &hKey);
    if (lOpenStatus == ERROR_SUCCESS) 
    {
        LSTATUS lSetStatus = RegSetValueEx(hKey, value, 0, REG_SZ,(LPBYTE)data, strlen(data) + 1);
        RegCloseKey(hKey);

        if (lSetStatus == ERROR_SUCCESS)
        {
            SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_BLOCK, 100, NULL);
            return TRUE;
        }
    }

    return FALSE;
}

gchar*
GetEnvironmentVariableWithKey(gchar* key)
{
    gchar* value = malloc(100);
    memset(value,0,100);

    LPTSTR lpszVariable; 
    LPTCH lpvEnv; 
 
    lpvEnv = GetEnvironmentStrings();
    
    lpszVariable = (LPTSTR) lpvEnv;
    while (*lpszVariable)
    {
        gchar* desire_prefix = "AGENT_PORT=";
        gchar* temp = lpszVariable;
        if(g_str_has_prefix(temp,desire_prefix))
        {
            gchar* after = temp + strlen(desire_prefix);
            memcpy(value,after,strlen(after));
            return value;
        }
        lpszVariable += lstrlen(lpszVariable) + 1;
    }
    FreeEnvironmentStrings(lpvEnv);
}
#endif

typedef enum _ReturnCode 
{
    PORT_FORWARD_OK = 100,
    ERROR_GET_ENV,
    ERROR_FETCH_INSTANCE_INFOR,
    ERROR_INIT_SSH_CLIENT,
    ERROR_CONNECT_TO_INSTANCE,
    ERROR_PORTFORWARD,
    ERROR_HANDLE_SSH_CONNECTION,
    ERROR_DOTNET_ENVIRONMENT = 131,
}ReturnCode;

struct _PortForward
{
    /**
     * @brief 
     * 
     */
    ChildProcess* process;

    /**
     * @brief 
     * port exposed to worker manager
     */
    gchar port[20];
};



gchar*
portforward_get_agent_instance_port(PortForward *port)
{
    return port->port;
}

PortForward*
init_portforward_service()
{
    PortForward* port = malloc(sizeof(PortForward));
    memset(port,0,sizeof(PortForward));

    

    gchar *buffer = GetEnvironmentVariableWithKey("AGENT_PORT");
    memcpy(port->port,buffer,strlen(buffer));
    free(buffer);

    if(!strlen(port))
    {
        SoupMessage* agent_request = soup_message_new(SOUP_METHOD_GET,PORT_OBTAIN_URL);
        soup_message_headers_append(agent_request->request_headers,"Authorization",CLUSTER_TOKEN);
        soup_message_set_request(agent_request,"application/json", SOUP_MEMORY_COPY, "",0);


        const gchar* https_aliases[] = { "https", NULL };
        SoupSession* host_session = soup_session_new_with_options(
                SOUP_SESSION_SSL_STRICT, FALSE,
                SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
                SOUP_SESSION_HTTPS_ALIASES, https_aliases, NULL);
        soup_session_send_message(host_session,agent_request);

        GError* error;
        JsonParser* parser = json_parser_new();
        JsonObject* object = get_json_object_from_string(agent_request->response_body->data,&error,parser);
        gint instancePort = json_object_get_int_member(object,"instancePort");
        itoa(instancePort,port->port,10);
        SetPermanentEnvironmentVariable("AGENT_PORT",port->port);
        g_object_unref(parser);
    }

    return port;
}




static void
handle_portforward_disconnected(ChildProcess* proc,
                                AgentServer* agent)
{
    PortForward* port = agent_get_portforward(agent);
    memset(port->port,0,20);  
    gint exit_code = childprocess_get_exit_code(proc);

    switch (exit_code)
    {
        case PORT_FORWARD_OK :
            worker_log_output("Portforward session ended");
            break;
        case ERROR_GET_ENV:
            worker_log_output("Error get environment variable");
            break;
        case ERROR_FETCH_INSTANCE_INFOR:
            worker_log_output("Error fetch instance infor");
            break;
        case ERROR_INIT_SSH_CLIENT:
            worker_log_output("Error init SSH client");
            break;
        case ERROR_CONNECT_TO_INSTANCE:
            worker_log_output("Error establish instance SSH connection");
            break;
        case ERROR_PORTFORWARD:
            worker_log_output("Error start portforward");
            break;
        case ERROR_HANDLE_SSH_CONNECTION:
            worker_log_output("Error handle SSH connection");
            break;
        case ERROR_DOTNET_ENVIRONMENT :
            worker_log_output("Dotnet environment is missing");
            break;
        default:
            worker_log_output("Unknown error while portforward");
            break;
    }


    while (!start_portforward(agent))
    {
#ifdef G_OS_WIN32
        Sleep(1000);
#else
        sleep(1000);
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
    gchar* agent_port = portforward_get_agent_instance_port(port);

#ifdef G_OS_WIN32
    SetEnvironmentVariable("port", TEXT(agent_port));
    SetEnvironmentVariable("clustertoken", TEXT(CLUSTER_TOKEN));
    SetEnvironmentVariable("clusterinfor", TEXT(CLUSTER_INFOR));
#endif

    // return false if session core is running before the initialization
    port->process = create_new_child_process(PORT_FORWARD_BINARY,
        (ChildStdErrHandle)handle_portfoward_error,
        (ChildStdOutHandle)handle_portforward_output,
        (ChildStateHandle)handle_portforward_disconnected, agent,NULL);

    return port->process ? port : NULL;
}




