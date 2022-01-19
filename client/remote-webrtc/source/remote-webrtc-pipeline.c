/**
 * @file remote-webrtc-pipeline.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <remote-webrtc-pipeline.h>
#include <remote-webrtc-type.h>
#include <remote-webrtc-data-channel.h>
#include <remote-webrtc-signalling.h>
#include <remote-webrtc-remote-config.h>
#include <remote-webrtc-pipeline.h>
#include <remote-webrtc-gui.h>
#include <remote-webrtc-input.h>

#include <qoe.h>

#include <gst/gst.h>
#include <glib-2.0/glib.h>
#include <gst/webrtc/webrtc.h>




/**
 * @brief 
 * naming of gstelement
 */
enum
{
    /**
     * @brief 
     * 
     */
    VIDEO_SINK,
    VIDEO_QUEUE_SINK,

    /**
     * @brief 
     * 
     */
    VIDEO_CONVERT,
    VIDEO_QUEUE_CONVERT,

    /**
     * @brief 
     * 
     */
    VIDEO_DECODER,
    VIDEO_QUEUE_DECODER,

    /**
     * @brief 
     * 
     */
    VIDEO_DEPAYLOAD,
    VIDEO_QUEUE_DEPAYLOAD,

    VIDEO_ELEMENT_LAST
};

/**
 * @brief 
 * naming of gstelement
 */
enum
{
    AUDIO_SINK,
    AUDIO_QUEUE_SINK,

    /**
     * @brief 
     * 
     */
    AUDIO_RESAMPLE,
    AUDIO_QUEUE_RESAMPLE,

    /**
     * @brief 
     * 
     */
    AUDIO_CONVERT,
    AUDIO_QUEUE_CONVERT,


    /**
     * @brief 
     * 
     */
    AUDIO_DECODER,
    AUDIO_QUEUE_DECODER,

    /**
     * @brief 
     * 
     */
    AUDIO_DEPAYLOAD,
    AUDIO_QUEUE_DEPAYLOAD,

    AUDIO_ELEMENT_LAST
};


struct _Pipeline
{
    GstElement* pipeline;
    GstElement* webrtcbin;

    GstElement* video_element[VIDEO_ELEMENT_LAST];
    GstElement* audio_element[AUDIO_ELEMENT_LAST];
};


void
setup_video_sink_navigator(RemoteApp* core);



Pipeline*
pipeline_initialize(RemoteApp* core)
{
    Pipeline* pipeline = malloc(sizeof(Pipeline));
    memset(pipeline,0,sizeof(Pipeline));
    return pipeline;
}

void
free_pipeline(Pipeline* pipeline)
{
    gst_element_set_state (pipeline->pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline->pipeline);
    memset(pipeline,0,sizeof(Pipeline));
}


static gboolean
start_pipeline(RemoteApp* core)
{
    GstStateChangeReturn ret;
    Pipeline* pipe = remote_app_get_pipeline(core);

    ret = GST_IS_ELEMENT(pipe->pipeline);    

    ret = gst_element_set_state(GST_ELEMENT(pipe->pipeline), GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        GError error;
        error.message = "Fail to start pipeline, this may due to pipeline setup failure";
        remote_app_finalize(core, &error);
    }
    return TRUE;
}



static void
handle_audio_stream (GstPad * pad, 
                     RemoteApp* core)
{
    Pipeline* pipeline = remote_app_get_pipeline(core);

    pipeline->audio_element[AUDIO_CONVERT] = gst_element_factory_make ("audioconvert", NULL);
    pipeline->audio_element[AUDIO_RESAMPLE]= gst_element_factory_make ("audioresample", NULL);
    pipeline->audio_element[AUDIO_SINK] =    gst_element_factory_make ("autoaudiosink", NULL);

    /* Might also need to resample, so add it just in case.
    * Will be a no-op if it's not required. */
    gst_bin_add_many (GST_BIN (pipeline->pipeline), 
        pipeline->audio_element[AUDIO_CONVERT], 
        pipeline->audio_element[AUDIO_RESAMPLE], 
        pipeline->audio_element[AUDIO_SINK], NULL);

    gst_element_sync_state_with_parent (pipeline->audio_element[AUDIO_CONVERT]);
    gst_element_sync_state_with_parent (pipeline->audio_element[AUDIO_RESAMPLE]);
    gst_element_sync_state_with_parent (pipeline->audio_element[AUDIO_SINK]);

    gst_element_link_many ( 
        pipeline->audio_element[AUDIO_QUEUE_CONVERT], 
        pipeline->audio_element[AUDIO_CONVERT], 
        pipeline->audio_element[AUDIO_QUEUE_RESAMPLE],
        pipeline->audio_element[AUDIO_RESAMPLE],
        pipeline->audio_element[AUDIO_QUEUE_SINK],
        pipeline->audio_element[AUDIO_SINK], NULL);

    GstPad* queue_pad = gst_element_get_static_pad (pipeline->audio_element[AUDIO_QUEUE_CONVERT], "sink");
    GstPadLinkReturn ret = gst_pad_link (pad, queue_pad);
    g_assert_cmphex (ret, ==, GST_PAD_LINK_OK);
}

static void
handle_video_stream (GstPad * pad, 
                     RemoteApp* core)
{
    Pipeline* pipeline = remote_app_get_pipeline(core);

    pipeline->video_element[VIDEO_CONVERT] = gst_element_factory_make ("videoconvert", NULL);
    pipeline->video_element[VIDEO_SINK] = gst_element_factory_make ("d3d11videosink", NULL);

    gst_bin_add_many (GST_BIN (pipeline->pipeline),
        pipeline->video_element[VIDEO_CONVERT], 
        pipeline->video_element[VIDEO_SINK], NULL);

    gst_element_sync_state_with_parent (pipeline->video_element[VIDEO_CONVERT]);
    gst_element_sync_state_with_parent (pipeline->video_element[VIDEO_SINK]);

    gst_element_link_many ( 
        pipeline->video_element[VIDEO_QUEUE_CONVERT], 
        pipeline->video_element[VIDEO_CONVERT], 
        pipeline->video_element[VIDEO_QUEUE_SINK], 
        pipeline->video_element[VIDEO_SINK], NULL);

#ifndef G_OS_WIN32
    setup_video_sink_navigator(core);
#endif
    
    GstPad* queue_pad = gst_element_get_static_pad (pipeline->video_element[VIDEO_QUEUE_CONVERT], "sink");
    GstPadLinkReturn ret = gst_pad_link (pad, queue_pad);
    g_assert_cmphex (ret, ==, GST_PAD_LINK_OK);

    trigger_capture_input_event(core);
    setup_video_overlay(pipeline->video_element[VIDEO_SINK],core);
}



/**
 * @brief 
 * 
 * @param decodebin 
 * @param pad 
 * @param data 
 */
static void
on_incoming_decodebin_stream (GstElement * decodebin, 
                              GstPad * pad,
                              gpointer data)
{
    RemoteApp* core = (RemoteApp*)data;
    Pipeline* pipeline = remote_app_get_pipeline(core);

    if (!gst_pad_has_current_caps (pad)) 
    {
        g_printerr ("Pad '%s' has no caps, can't do anything, ignoring\n",
            GST_PAD_NAME (pad));
        return;
    }

    GstCaps* caps = gst_pad_get_current_caps (pad);
    gchar*   name = gst_structure_get_name (gst_caps_get_structure (caps, 0));

    if (g_str_has_prefix (name, "video")) 
    {
        handle_video_stream(pad, core);
    } 
    else if (g_str_has_prefix (name, "audio")) 
    {
        handle_audio_stream(pad, core);
    } 
    else 
    {
      g_printerr ("Unknown pad %s, ignoring", GST_PAD_NAME (pad));
    }
}



/**
 * @brief 
 * 
 * @param webrtc 
 * @param webrtcbin_pad 
 * @param data 
 */
static void
on_incoming_stream (GstElement * webrtc, 
                    GstPad * webrtcbin_pad, 
                    gpointer data)
{
    RemoteApp* core = (RemoteApp*)data;
    if (GST_PAD_DIRECTION (webrtcbin_pad) != GST_PAD_SRC)
      return;

    Pipeline* pipeline = remote_app_get_pipeline(core);
    
    GstCaps* caps = gst_pad_get_current_caps(webrtcbin_pad);
    gchar* encoding = gst_structure_get_string(gst_caps_get_structure(caps, 0), "encoding-name");
    gchar* name = gst_structure_get_name(gst_caps_get_structure(caps, 0));

    if(!g_strcmp0("application/x-rtp",name) &&
       !g_strcmp0("OPUS",encoding))
    {
        pipeline->audio_element[AUDIO_DECODER] = gst_element_factory_make ("decodebin", "audiodecoder");
        g_signal_connect (pipeline->audio_element[AUDIO_DECODER], "pad-added",
            G_CALLBACK (on_incoming_decodebin_stream), core);
        gst_bin_add (GST_BIN (pipeline->pipeline), pipeline->audio_element[AUDIO_DECODER]);

        gst_element_sync_state_with_parent (pipeline->audio_element[AUDIO_DECODER]);

        GstCaps* cap = gst_element_get_static_pad (pipeline->audio_element[AUDIO_DECODER], "sink");
        gst_pad_link (webrtcbin_pad, cap);
        gst_object_unref (cap);
    }

    if(!g_strcmp0("application/x-rtp",name) &&
       (!g_strcmp0("H265",encoding) ||
        !g_strcmp0("H264",encoding)))
    {
        pipeline->video_element[VIDEO_DECODER] = gst_element_factory_make ("decodebin", "videodecoder");
        g_signal_connect (pipeline->video_element[VIDEO_DECODER], "pad-added",
            G_CALLBACK (on_incoming_decodebin_stream), core);
        gst_bin_add (GST_BIN (pipeline->pipeline), pipeline->video_element[VIDEO_DECODER]);

        gst_element_sync_state_with_parent (pipeline->video_element[VIDEO_DECODER]);

        GstCaps* cap = gst_element_get_static_pad (pipeline->video_element[VIDEO_DECODER], "sink");
        gst_pad_link (webrtcbin_pad, cap);
        gst_object_unref (cap);
    }
}











#ifndef G_OS_WIN32

static gboolean
handle_event(GstPad* pad, 
            GstObject* parent, 
            GstEvent* event)
{
    switch (GST_EVENT_TYPE (event)) {
      case GST_EVENT_NAVIGATION:
        // handle_navigator(event,pipeline_singleton.core);
        break;
      default:
        gst_pad_event_default(pad, parent,event);
        break;
    }
}

/**
 * @brief Set the up video sink navigator object
 * 
 * @param core 
 */
void
setup_video_sink_navigator(RemoteApp* core)
{
    Pipeline* pipeline = remote_app_get_pipeline(core);
    GstPad* pad = gst_element_get_static_pad(pipeline->video_element[VIDEO_CONVERT],"src");

    gst_pad_set_event_function_full(pad,handle_event,core,NULL);
}
#endif

 
static void
setup_pipeline_queue(Pipeline* pipeline)
{
    GstElement* queue_array[9];
    for (gint i = 0; i < 9; i++)
    {
        queue_array[i] = gst_element_factory_make ("queue", NULL);
        g_object_set(queue_array[i], "max-size-time", 0, NULL);
        g_object_set(queue_array[i], "max-size-bytes", 0, NULL);
        g_object_set(queue_array[i], "max-size-buffers", 3, NULL);

        gst_bin_add(GST_BIN(pipeline->pipeline),queue_array[i]);
        gst_element_sync_state_with_parent(queue_array[i]);
    }

    pipeline->audio_element[AUDIO_QUEUE_SINK] =             queue_array[0];
    pipeline->audio_element[AUDIO_QUEUE_RESAMPLE] =         queue_array[1];
    pipeline->audio_element[AUDIO_QUEUE_CONVERT] =          queue_array[2];
    pipeline->audio_element[AUDIO_QUEUE_DECODER] =          queue_array[3];
    pipeline->audio_element[AUDIO_QUEUE_DEPAYLOAD] =        queue_array[4];

    pipeline->video_element[VIDEO_QUEUE_SINK] =             queue_array[5];
    pipeline->video_element[VIDEO_QUEUE_CONVERT] =          queue_array[6];
    pipeline->video_element[VIDEO_QUEUE_DECODER] =          queue_array[7];
    pipeline->video_element[VIDEO_QUEUE_DEPAYLOAD] =        queue_array[8];
}


gpointer
setup_pipeline(RemoteApp* core)
{
    GstCaps *video_caps;
    GstWebRTCRTPTransceiver *trans = NULL;
    SignallingHub* signalling = remote_app_get_signalling_hub(core);
    Pipeline* pipe = remote_app_get_pipeline(core);

    if(pipe->pipeline)
        free_pipeline(pipe);

    GError* error = NULL;

    pipe->pipeline = gst_parse_launch("webrtcbin name=webrtcbin  bundle-policy=max-bundle audiotestsrc is-live=true wave=red-noise ! audioconvert ! audioresample ! queue ! opusenc ! rtpopuspay ! queue ! application/x-rtp,media=audio,payload=96,encoding-name=97 ! webrtcbin",&error);
    pipe->webrtcbin =  gst_bin_get_by_name(GST_BIN(pipe->pipeline),"webrtcbin");
    g_object_set(pipe->webrtcbin, "latency", 0, NULL);

    setup_pipeline_queue(pipe);

    /* Incoming streams will be exposed via this signal */
    g_signal_connect(pipe->webrtcbin, "pad-added",
        G_CALLBACK (on_incoming_stream),core);

    GstStateChangeReturn result = gst_element_change_state(pipe->pipeline, GST_STATE_READY);
    if(result == GST_STATE_CHANGE_FAILURE)
    {
        g_print("remote app fail to start, aborting ...\n");
        remote_app_finalize(core,NULL);
    }
    connect_signalling_handler(core);
    connect_data_channel_signals(core);
    start_pipeline(core);
}







GstElement*
pipeline_get_webrtc_bin(Pipeline* pipe)
{
    return pipe->webrtcbin;
}

GstElement*
pipeline_get_pipline(Pipeline* pipe)
{
    return pipe->pipeline;
}



GstElement*         
pipeline_get_pipeline_element(Pipeline* pipeline)
{
    return pipeline->pipeline;
}