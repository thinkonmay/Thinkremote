
#include <glib-2.0/glib.h>
#include <gst/gst.h>

#include <shortcut.h>
#include <handle-key.h>
#include <capture-key.h>
#include <overlay-gui.h>



void            
print_out(gchar* message,
         gpointer data)
{
    g_print("%s \n ",message);
}

int 
main()
{
    GstElement* pipeline = gst_parse_launch("d3d11screencapturesrc name=source ! queue ! d3d11videosink name=sink");
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

    g_main_loop_new()
}