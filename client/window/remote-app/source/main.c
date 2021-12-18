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
#include <remote-app.h>
#include <remote-app-type.h>

#include <gst/gst.h>
#include <glib-2.0/glib.h>
#include <development.h>

#define GST_USE_UNSTABLE_API

static gchar connection_string[500] = {0};
static gchar remote_token[500] = {0};


#define GST_DEBUG               4

static GOptionEntry entries[] = {
    {"url", 0, 0, G_OPTION_ARG_STRING, &connection_string,
        "url run by electron app to initialize remote app", "codec"},
    {NULL},
};


char**
split(char *string, 
      const char delimiter) 
{
    int length = 0, count = 0, i = 0, j = 0;
    while(*(string++)) {
        if (*string == delimiter) count++;
        length++;
    }
    string -= (length + 1); // string was incremented one more than length
    char **array = (char **)malloc(sizeof(char *) * (length + 1));
    char ** base = array;
    for(i = 0; i < (count + 1); i++) {
        j = 0;
        while(string[j] != delimiter) j++;
        j++;
        *array = (char *)malloc(sizeof(char) * j);
        memcpy(*array, string, (j-1));
        (*array)[j-1] = '\0';
        string += j;
        array++;
    }
    *array = '\0';
    return base;  
}

void
string_split_free(gchar** base)
{
    gint i = 0;
    while(base[i]) {
        free(base[i]);
        i++;
    }
    free(base);
    base = NULL;
}


int
main(int argc, char* argv[])
{
    GOptionContext *context;
    GError *error = NULL;

    context = g_option_context_new ("- thinkmay gstreamer client");
    g_option_context_add_main_entries (context, entries, NULL);
    g_option_context_add_group (context, gst_init_get_option_group ());
    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_printerr ("Error initializing: %s\n", error->message);
        return -1;
    }


    if(!DEVELOPMENT_ENVIRONMENT)
    {
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
    }
    else
    {
        g_print("Starting development client\n");
        memcpy(remote_token,DEFAULT_CLIENT_TOKEN,strlen(DEFAULT_CLIENT_TOKEN));
    }


    remote_app_initialize(remote_token);
    return 0;
}
















