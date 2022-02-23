/**
 * @file main.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <remote-webrtc.h>
#include <remote-webrtc-type.h>

#include <gst/gst.h>
#include <glib-2.0/glib.h>
#include <constant.h>
#include <string-manipulate.h>
#include <global-var.h>



static gchar remote_token[500] = {0};

/**
 * @brief 
 * 
 */
const gchar* environment;


static GOptionEntry entries[] = {
    {"environment", 0, 0, G_OPTION_ARG_STRING, &environment,
        "environment (dev = development, default = production)", "ENV"},
    {NULL},
};




int
main(int argc, char* argv[])
{
    environment = malloc(100);
    memset(environment,0,100);

    GOptionContext *context;
    GError *error = NULL;

    context = g_option_context_new ("- thinkremote");
    g_option_context_add_main_entries (context, entries, NULL);
    g_option_context_add_group (context, gst_init_get_option_group ());
    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_printerr ("Error initializing: %s\n", error->message);
        return -1;
    }

    thinkremote_application_init(environment,
                                NULL,
                                NULL,
                                NULL);

    if(DEVELOPMENT_ENVIRONMENT)
        goto start;

    if(argc == 2)
    {
        gchar** array = split(argv[1],'/');
        if(g_strcmp0(array[0],"thinkmay:"))
        {
            g_print("%s :",array[0]);
            g_printerr("wrong uri, remote app exiting");
            return;
        }

        gchar** parameter = split(array[2],'=');
        if(!g_strcmp0(*(parameter ),"token"))
        {
            memcpy(remote_token,parameter[1],strlen(parameter[1]));
        }
    }

start:
    remote_app_initialize(remote_token);
    return 0;
}
















