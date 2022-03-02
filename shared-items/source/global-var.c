/**
 * @file global-var.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <global-var.h>
#include <environment.h>
#include <constant.h>
#include <glib.h>

#ifdef G_OS_WIN32
#include <Windows.h>
#endif


static gboolean env                  = FALSE;
static gboolean is_localhost         = FALSE;
static gchar _cluster_url[500]       = {0}; 
static gchar _device_token[500]      = {0}; 
static gchar _cluster_token[500]     = {0}; 


void
thinkremote_application_init(gchar* environment,
                             gchar* cluster_url,
                             gchar* cluster_token,
                             gchar* device_token)
{
    gchar* localhost;
#ifdef G_OS_WIN32
    SetEnvironmentVariable("GST_DEBUG", TEXT("0"));
    localhost = GetEnvironmentVariableWithKey("LOCALHOST");
#endif

    if(!g_strcmp0(environment,"development") ||
       !g_strcmp0(environment,"localhost"))
        env = TRUE;

    if(!g_strcmp0(environment,"localhost") ||
       !g_strcmp0(localhost,"TRUE"))
    {
        SetEnvironmentVariable("LOCALHOST", TEXT("TRUE"));
        is_localhost = TRUE;
    }

    if(cluster_url)
        memcpy(_cluster_url,cluster_url,strlen(cluster_url));

    if(cluster_token)
        memcpy(_cluster_token,cluster_token,strlen(cluster_token));

    if(device_token)
        memcpy(_device_token,device_token,strlen(device_token));
}

void            
update_device_token(gchar* device_token)
{
    memcpy(_device_token,device_token,strlen(device_token));
}

gchar*                      
get_thinkremote_cluster_ip() 
{
    return _cluster_url;
}
gchar* 
get_thinkremote_device_token() 
{
    return _device_token;
}
gchar*                     
get_thinkremote_cluster_token() 
{
    return _cluster_token;
}

gboolean 
get_environment()
{
    return env;
}

gboolean 
is_localhost_env()
{
    return is_localhost;
}
