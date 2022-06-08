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

#ifdef G_OS_WIN32
#include <shortcut.h>
#include <capture-key.h>
#include <overlay-gui.h>
#include <handle-key.h>
#endif


void            
print_out(gchar* message,
         gpointer data)
{
    g_print("%s \n ",message);
}


int 
main()
{
#ifdef G_OS_WIN32
    gst_init(NULL,NULL);

    GError* error = NULL;
    GstElement* pipeline =    gst_parse_launch("videotestsrc name=source ! queue ! d3d11videosink name=sink",&error);

    GstElement* videosource = gst_bin_get_by_name(pipeline,"source");
    GstElement* videosink   = gst_bin_get_by_name(pipeline,"sink");

    GstPad* pad = gst_element_get_static_pad (videosink, "sink");
    GstCaps* caps = gst_pad_get_current_caps (pad);
    if (!caps)
        caps = gst_pad_query_caps (pad, NULL);

    Shortcut* shortcuts = shortcut_list_initialize(10);
    GUI* gui = init_remote_app_gui(NULL,shortcuts,print_out);
    setup_video_overlay(gui,
        caps,
        videosink,
        pipeline);

    gint result = gst_element_change_state(pipeline,GST_STATE_PLAYING);
    if(result == GST_STATE_CHANGE_FAILURE)
        return;

    GMainLoop* loop = g_main_loop_new(NULL,FALSE);
    g_main_loop_run(loop);
#endif
}