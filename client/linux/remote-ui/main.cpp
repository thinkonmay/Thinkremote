#include <remote-app-type.h>
#include "remoteui.h"
#include <QApplication>

#include <QMediaPlayer>
#include <QVideoWidget>
#include <QHBoxLayout>
#include <QMediaPlaylist>
#include <glib-2.0/glib.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <QTimer>
#include <QWidget>

// #include <gst/gst.h>
// #include <glib-2.0/glib.h>

// #include <string-spliter.h>
// #include <general-constant.h>

// #ifndef GST_USE_UNSTABLE_API
// #define GST_USE_UNSTABLE_API
// #endif

// static gchar signalling_url[50] = "wss://host.thinkmay.net/Handshake";
// static gchar turn[100] = "turn://thinkmaycoturn:thinkmaycoturn_password@turn:stun.thinkmay.net:3478";
// static gint  session_id = 0;
// static gchar video_codec[50] = {0};
// static gchar audio_codec[50] = {0}; 
// static gchar connection_string[200] = {0};


// #define GST_DEBUG               4

// static GOptionEntry entries[] = {
//   {"sessionid", 0, 0, G_OPTION_ARG_INT, &session_id,
//       "String ID of the peer to connect to", "ID"},
//   {"signalling", 0, 0, G_OPTION_ARG_STRING, &signalling_url,
//       "Signalling server to connect to", "URL"},
//   {"turn", 0, 0, G_OPTION_ARG_STRING, &turn,
//       "Request that the peer generate the offer and we'll answer", "URL"},
//   {"audiocodec", 0, 0, G_OPTION_ARG_STRING, &audio_codec,
//       "audio codec use for decode bin", "codec"},
//   {"videocodec", 0, 0, G_OPTION_ARG_STRING, &video_codec,
//       "video codec use for decode bin", "codec"},
//   {"connection", 0, 0, G_OPTION_ARG_STRING, &connection_string,
//       "connection ", "codec"},
//   {NULL},
// };


// char**
// split(char *string, 
//       const char delimiter) 
// {
//     int length = 0, count = 0, i = 0, j = 0;
//     while(*(string++)) {
//         if (*string == delimiter) count++;
//         length++;
//     }
//     string -= (length + 1); // string was incremented one more than length
//     char **array = (char **)malloc(sizeof(char *) * (length + 1));
//     char ** base = array;
//     for(i = 0; i < (count + 1); i++) {
//         j = 0;
//         while(string[j] != delimiter) j++;
//         j++;
//         *array = (char *)malloc(sizeof(char) * j);
//         memcpy(*array, string, (j-1));
//         (*array)[j-1] = '\0';
//         string += j;
//         array++;
//     }
//     *array = '\0';
//     return base;  
// }

// void
// string_split_free(gchar** base)
// {
//     gint i = 0;
//     while(base[i]) {
//         free(base[i]);
//         i++;
//     }
//     free(base);
//     base = NULL;
// }


/* slightly convoluted way to find a working video sink that's not a bin,
 * one could use autovideosink from gst-plugins-good instead
 */
//static GstElement *
//find_video_sink (void)
//{
//  GstStateChangeReturn sret;
//  GstElement *sink;

//  if ((sink = gst_element_factory_make ("xvimagesink", NULL))) {
//    sret = gst_element_set_state (sink, GST_STATE_READY);
//    if (sret == GST_STATE_CHANGE_SUCCESS)
//      return sink;

//    gst_element_set_state (sink, GST_STATE_NULL);
//  }
//  gst_object_unref (sink);

//  if ((sink = gst_element_factory_make ("ximagesink", NULL))) {
//    sret = gst_element_set_state (sink, GST_STATE_READY);
//    if (sret == GST_STATE_CHANGE_SUCCESS)
//      return sink;

//    gst_element_set_state (sink, GST_STATE_NULL);
//  }
//  gst_object_unref (sink);

//  if (strcmp ("autovideosink", "xvimagesink") == 0 ||
//      strcmp ("autovideosink", "ximagesink") == 0)
//    return NULL;

//  if ((sink = gst_element_factory_make ("autovideosink", NULL))) {
//    if (GST_IS_BIN (sink)) {
//      gst_object_unref (sink);
//      return NULL;
//    }

//    sret = gst_element_set_state (sink, GST_STATE_READY);
//    if (sret == GST_STATE_CHANGE_SUCCESS)
//      return sink;

//    gst_element_set_state (sink, GST_STATE_NULL);
//  }
//  gst_object_unref (sink);
//  return NULL;
//}


int main(int argc, char* argv[])
{

   gst_init (&argc, &argv);
   QApplication app(argc, argv);
   app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit ())); //get window close

   // prepare the pipeline

   GstElement *pipeline = gst_pipeline_new ("xvoverlay");
   GstElement *src = gst_element_factory_make ("videotestsrc", NULL);
   GstElement *sink = gst_element_factory_make ("xvimagesink", NULL);



   gst_bin_add_many (GST_BIN (pipeline), src, sink, NULL);
   gst_element_link (src, sink);



   //////////////////////////////

   // prepare the ui

   QWidget window;
   window.resize(320, 240);
   window.show();

   WId xwinid = window.winId(); // get window id


   //////////////////////////////////////////////////////////////////////////////
   gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (sink), xwinid);
   //////////////////////////////////////////////////////////////////////////////


   // run the pipeline

   GstStateChangeReturn sret = gst_element_set_state (pipeline,
       GST_STATE_PLAYING);
   if (sret == GST_STATE_CHANGE_FAILURE) {
     gst_element_set_state (pipeline, GST_STATE_NULL);
     gst_object_unref (pipeline);
     // Exit application
     QTimer::singleShot(0, QApplication::activeWindow(), SLOT(quit()));
   }

   int ret = app.exec();

   window.hide();
   gst_element_set_state (pipeline, GST_STATE_NULL);
   gst_object_unref (pipeline);

   return ret;
   ///////////////////////////////////////////////////////////////////



    thinkmay_init(argv[0],19);

    GOptionContext *context;
    GError *error = NULL;

    context = g_option_context_new ("- thinkmay gstreamer client");
    g_option_context_add_main_entries (context, entries, NULL);
    g_option_context_add_group (context, gst_init_get_option_group ());
    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_printerr ("Error initializing: %s\n", error->message);
        return -1;
    }

    if(argc == 2)
    {
        gchar** array = split(argv[1],'/');

        if(g_strcmp0(array[0],"thinkmay:"))
        {
            g_print("%s :",array[0]);
            g_printerr("wrong uri, remote app exiting");
            return;
        }


        gchar** array_param = split(array[2],'.');
        do
        {
            if(*(array_param))
            {
                gchar** parameter = split(*(array_param),'=');
                if(!g_strcmp0(*(parameter ),"sessionid"))
                {
                    gint id = strtol(*(parameter +1),NULL,10);
                    session_id = id;
                }
                else if(!g_strcmp0(*(parameter ),"signalling"))
                {
                    memcpy(signalling_url,*(parameter +1),strlen(*(parameter +1)));
                }
                else if(!g_strcmp0(*(parameter ),"turn"))
                {
                    memcpy(turn,*(parameter +1),strlen(*(parameter +1)));
                }
                else if(!g_strcmp0(*(parameter ),"videocodec"))
                {
                    memcpy(video_codec,*(parameter +1),strlen(*(parameter +1)));
                }
            }
        }
        while(*(array_param++));
    }

    g_print("Starting connnection with session client id %d, videocodec %s , signalling server url %s\n",session_id,video_codec,signalling_url);

    gst_init (&argc, &argv);
     QApplication app(argc, argv);
     app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit ()));

<<<<<<< HEAD
     /* prepare the pipeline */
=======
    QApplication a(argc, argv);
     RemoteUI w;
     w.show();
>>>>>>> 64118f2781fb94383e30cddf8e6ecaac9a290be2

     GstElement *pipeline = gst_pipeline_new ("xvoverlay");
     GstElement *src = gst_element_factory_make ("videotestsrc", NULL);
     GstElement *sink = gst_element_factory_make ("playsink", NULL);
     gst_bin_add_many (GST_BIN (pipeline), src, sink, NULL);
     gst_element_link (src, sink);

     if (sink == NULL)
       g_error ("Couldn't find a working video sink.");

<<<<<<< HEAD
     /* prepare the ui */

     QWidget window;
     window.resize(320, 240);
     window.setWindowTitle("GstVideoOverlay Qt demo");
     window.show();
=======
>>>>>>> 64118f2781fb94383e30cddf8e6ecaac9a290be2

     WId xwinid = window.winId();
     gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (sink), xwinid);

     /* run the pipeline */

     GstStateChangeReturn sret = gst_element_set_state (pipeline,
         GST_STATE_PLAYING);
     if (sret == GST_STATE_CHANGE_FAILURE) {
       gst_element_set_state (pipeline, GST_STATE_NULL);
       gst_object_unref (pipeline);
       /* Exit application */
       QTimer::singleShot(0, QApplication::activeWindow(), SLOT(quit()));
     }

     int ret = app.exec();

     window.hide();
     gst_element_set_state (pipeline, GST_STATE_NULL);
     gst_object_unref (pipeline);

<<<<<<< HEAD
     return ret;
=======
    remote_app_initialize(session_id,
                    signalling_url, 
                    turn, 
                    audio_codec, 
                    video_codec);
    return 0;
>>>>>>> 64118f2781fb94383e30cddf8e6ecaac9a290be2
}
