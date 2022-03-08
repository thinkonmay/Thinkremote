/**
 * @file key.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2022-03-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <glib-2.0/glib.h>
#include <gst/gst.h>

#include <shortcut.h>
#include <handle-key.h>
#include <capture-key.h>
#include <overlay-gui.h>
#include <key-convert.h>

#include <Windows.h>
GMainLoop* loop;

void do_nothing(){}

static gint testlist[100] = 
{
    VK_UP,
    VK_DOWN,
    VK_LEFT,
    VK_RIGHT,

    VK_DELETE,
    0

};

gpointer 
create_key_thread(gpointer data)
{
    Sleep(5000);

    gint i = 0;
    while (testlist[i])
    {
        JsonObject* object = json_object_new();
        json_object_set_int_member(object,"Opcode",KEYRAW);
        json_object_set_int_member(object,"wVk",testlist[i]);
        json_object_set_int_member(object,"keyFlags",RI_KEY_MAKE | RI_KEY_E0);
        gchar* down = get_string_from_json_object(object);

        object = json_object_new();
        json_object_set_int_member(object,"Opcode",KEYRAW);
        json_object_set_int_member(object,"wVk",testlist[i]);
        json_object_set_int_member(object,"keyFlags",RI_KEY_BREAK | RI_KEY_E0);
        gchar* up = get_string_from_json_object(object);

        handle_input_win32(down,do_nothing,NULL);
        Sleep(100);
        handle_input_win32(up,do_nothing,NULL);
        Sleep(1000);
        i++;
    }

    g_print("Key testing done");
    g_main_loop_quit(loop);
}

int 
main()
{
    gst_init(NULL,NULL);

    GError* error = NULL;
    GstElement* pipeline =    gst_parse_launch("d3d11screencapturesrc name=source ! queue ! fakesink name=sink",&error);

    GstElement* videosource = gst_bin_get_by_name(pipeline,"source");
    GstElement* videosink   = gst_bin_get_by_name(pipeline,"sink");
    
    Shortcut* shortcuts = shortcut_list_initialize(10);
    HIDHandler* handler = activate_hid_handler(videosource, 
                                               shortcuts);


    gint result = gst_element_change_state(pipeline,GST_STATE_PLAYING);
    if(result == GST_STATE_CHANGE_FAILURE)
        return;

    g_thread_new(NULL,create_key_thread,NULL);
    loop = g_main_loop_new(NULL,FALSE);
    g_main_loop_run(loop);
}