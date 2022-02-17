#include <session-udp.h>
#include <session-udp-type.h>

#include <development.h>

#include <gst/gst.h>
#include <glib-2.0/glib.h>
#include <global-var.h>

#define GST_USE_UNSTABLE_API



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

    GError *error = NULL;
    GOptionContext *context = g_option_context_new ("- thinkshare");
    g_option_context_add_main_entries (context, entries, NULL);
    g_option_context_add_group (context, gst_init_get_option_group ());
    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_printerr ("Error initializing: %s\n", error->message);
        return -1;
    }


    thinkremote_application_init(environment,NULL,NULL,NULL);
    if(!DEVELOPMENT_ENVIRONMENT)
    {
        g_print("session core start with cluster manager url\n");
        g_print(CLUSTER_URL);
        g_print("\n");
        g_print("session core start with worker token\n");
        g_print(DEVICE_TOKEN);
        g_print("\n");
    }
    else
    {
        g_print("Starting in development environment\n");
        
    }
    

    session_core_initialize();
    return;
}
