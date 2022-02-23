/**
 * @file remote-udp-pipeline.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <remote-udp-pipeline.h>
#include <remote-udp-type.h>
#include <remote-udp-remote-config.h>
#include <remote-udp-pipeline.h>

#include <enum.h>
#include <overlay-gui.h>

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
    VIDEO_DECODER,
    VIDEO_QUEUE_DECODER,

    /**
     * @brief 
     * 
     */
    UDP_VIDEO_SOURCE,

    VIDEO_ELEMENT_LAST
};

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
    UDP_AUDIO_SOURCE,

    AUDIO_ELEMENT_LAST
};


struct _Pipeline
{
    /**
     * @brief 
     * GstPipeline of the audio session
     */
	GstElement* audio_pipeline;

    /**
     * @brief 
     * GstPipeline for video stream 
     */
	GstElement* video_pipeline;


    GstElement* video_element[VIDEO_ELEMENT_LAST];
    GstElement* audio_element[AUDIO_ELEMENT_LAST];

    /**
     * @brief 
     * 
     */
    gint audio_port;

    /**
     * @brief 
     * 
     */
    gint video_port;
};


void
setup_video_sink_navigator(RemoteUdp* core);



Pipeline*
pipeline_initialize()
{
    Pipeline* pipeline = malloc(sizeof(Pipeline));
    memset(pipeline,0,sizeof(Pipeline));
    return pipeline;
}


void
setup_pipeline_startpoint(Pipeline* pipeline,
                          gint audio_port,
                          gint video_port)
{
    pipeline->audio_port = audio_port;
    pipeline->video_port = video_port;
}

void
free_pipeline(Pipeline* pipeline)
{
    if(pipeline->audio_pipeline)
    {
        gst_element_set_state (pipeline->audio_pipeline, GST_STATE_NULL);
        gst_object_unref (pipeline->audio_pipeline);
    }

    if(pipeline->video_pipeline)
    {
        gst_element_set_state (pipeline->video_pipeline, GST_STATE_NULL);
        gst_object_unref (pipeline->video_pipeline);
    }
    memset(pipeline,0,sizeof(Pipeline));
}


static gboolean
start_pipeline(GstElement* pipeline)
{
    GstStateChangeReturn ret;
    ret = gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
    return (ret != GST_STATE_CHANGE_FAILURE);
}


static void
handle_video_stream (GstElement* decodebin, 
                     GstPad* pad,
                     gpointer data)
{
    RemoteUdp* core = (RemoteUdp*) data; 
    Pipeline* pipeline = remote_app_get_pipeline(core);
    GUI* gui = remote_app_get_gui(core);

#ifdef G_OS_WIN32
    pipeline->video_element[VIDEO_SINK] = gst_element_factory_make ("d3d11videosink", NULL);
#else
    pipeline->video_element[VIDEO_SINK] = gst_element_factory_make ("ximagesink", NULL);
#endif

    gst_bin_add_many (GST_BIN (pipeline->video_pipeline), pipeline->video_element[VIDEO_SINK], NULL);

    gst_element_sync_state_with_parent (pipeline->video_element[VIDEO_SINK]);

    GstPad* queue_pad = gst_element_get_static_pad (pipeline->video_element[VIDEO_SINK], "sink");
    GstPadLinkReturn ret = gst_pad_link (pad, queue_pad);
    g_assert_cmphex (ret, ==, GST_PAD_LINK_OK);

    setup_video_overlay(gui,pipeline->video_element[VIDEO_SINK],pipeline->video_pipeline,core);
}

static void
handle_audio_stream(GstElement* decodebin, 
                     GstPad* pad,
                     gpointer data)
{
    RemoteUdp* core = (RemoteUdp*) data;
    Pipeline* pipeline = remote_app_get_pipeline(core);

    pipeline->audio_element[AUDIO_CONVERT] = gst_element_factory_make ("audioconvert", NULL);
    pipeline->audio_element[AUDIO_RESAMPLE]= gst_element_factory_make ("audioresample", NULL);
    pipeline->audio_element[AUDIO_SINK] =    gst_element_factory_make ("autoaudiosink", NULL);

    /* Might also need to resample, so add it just in case.
    * Will be a no-op if it's not required. */
    gst_bin_add_many (GST_BIN (pipeline->audio_pipeline), 
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



#ifdef G_OS_WIN32
#define DIRECTX_PAD "video/x-raw(memory:D3D11Memory)"
#endif

#define RTP_CAPS_AUDIO "application/x-rtp,media=audio,payload=96,encoding-name="
#define RTP_CAPS_VIDEO "application/x-rtp,media=video,payload=97,encoding-name="
#define QUEUE "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
static void
setup_element_factory(RemoteUdp* core,
                      Codec video, 
                      Codec audio)
{
    Pipeline* pipe = remote_app_get_pipeline(core);
    GError* error = NULL;
    
    if (video == CODEC_H264)
    {
        if (audio == OPUS_ENC) 
        {
            pipe->video_pipeline =
                gst_parse_launch(
                    "udpsrc name=udp ! "RTP_CAPS_VIDEO"H264 ! "                QUEUE
                    "rtph264depay ! "                                          QUEUE
                    "decodebin name=decoder",&error);
            pipe->audio_pipeline = 
                gst_parse_launch(
                    "udpsrc name=udp ! "RTP_CAPS_AUDIO"OPUS ! "                QUEUE
                    "rtpopusdepay ! "                                          QUEUE
                    "decodebin name=decoder",&error);
        }
    }
    else if (video == CODEC_H265)
    {
        if (audio == OPUS_ENC)
        {
            pipe->video_pipeline =
                gst_parse_launch(
                    "udpsrc name=udp ! "RTP_CAPS_VIDEO"H265 ! "                QUEUE
                    "rtph265depay ! "                                          QUEUE
                    "decodebin name=decoder",&error);
            pipe->audio_pipeline = 
                gst_parse_launch(
                    "udpsrc name=udp ! "RTP_CAPS_AUDIO"OPUS ! "                QUEUE
                    "rtpopusdepay ! "                                          QUEUE
                    "decodebin name=decoder",&error);

        }
    }


    if (error) 
        return; 

    pipe->audio_element[UDP_AUDIO_SOURCE] = 
        gst_bin_get_by_name(GST_BIN(pipe->audio_pipeline), "udp");
    pipe->audio_element[AUDIO_DECODER] = 
        gst_bin_get_by_name(GST_BIN(pipe->audio_pipeline), "decoder");

    pipe->video_element[UDP_VIDEO_SOURCE] = 
        gst_bin_get_by_name(GST_BIN(pipe->video_pipeline), "udp");
    pipe->video_element[VIDEO_DECODER] = 
        gst_bin_get_by_name(GST_BIN(pipe->video_pipeline), "decoder");
}

/**
 * @brief Set the up element property object
 * setup proerty of gst element,
 * this function should be called after pipeline factory has been done,
 * each element are assigned to an element in pipeline
 * @param core 
 */
static void
setup_element_property(RemoteUdp* core)
{
    Pipeline* pipe = remote_app_get_pipeline(core);
    RemoteConfig* qoe = remote_app_get_qoe(core);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef G_OS_WIN32
#else
#endif
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef G_OS_WIN32
#else
#endif
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef G_OS_WIN32
#else
#endif
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    g_object_set(pipe->video_element[UDP_VIDEO_SOURCE], "port", pipe->video_port, NULL);

    g_object_set(pipe->audio_element[UDP_AUDIO_SOURCE], "port", pipe->audio_port, NULL);
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

static gint
handle_audio_element(GstElement * bin,
                     GstPad * pad,
                     GstCaps * caps,
                     GstElementFactory * factory,
                     gpointer udata)
{
    gint i = 0;
    gboolean select = FALSE; 
    gchar** keys;
    keys = gst_element_factory_get_metadata_keys (factory);
    g_print("Querying new audio element\n");
    while (keys[i]) {
        gchar * value = gst_element_factory_get_metadata (factory,keys[i]);
        g_print("%s : %s\n",keys[i],value);
        i++;
    }

    g_print("\n\n");
    return 0;
}


static gint
handle_video_element(GstElement * bin,
                     GstPad * pad,
                     GstCaps * caps,
                     GstElementFactory * factory,
                     gpointer udata)
{
    gint i = 0;
    gboolean select = FALSE; 
    gchar** keys;
    keys = gst_element_factory_get_metadata_keys (factory);
    g_print("Querying new video element\n");
    while (keys[i]) {
        gchar * value = gst_element_factory_get_metadata (factory,keys[i]);
        g_print("%s : %s\n",keys[i],value);
        if (g_str_has_prefix(value,"RTP H264") ||
            g_str_has_prefix(value,"RTP H265")) {
            select = TRUE;
        } else if (g_str_has_prefix(value,"H.264 parser") || 
                   g_str_has_prefix(value,"H.265 parser")) {
            select = TRUE;
#ifdef G_OS_WIN32
        } else if (g_str_has_prefix(value,"Direct3D11")) {
            select = TRUE;
#else
        } else if (g_str_has_prefix(value,"libav")) {
            select = TRUE;
#endif 
        }

        i++;
    }

    g_print("\n\n");
    if(!select)
        return 2;
    else
        return 0;
}

static void
setup_pipeline_queue(Pipeline* pipeline)
{
    GstElement* audio_queue_array[4],* video_queue_array[4];
    for (gint i = 0; i < 4; i++)
    {
        audio_queue_array[i] = gst_element_factory_make ("queue", NULL);
        g_object_set(audio_queue_array[i], "max-size-time", 0, NULL);
        g_object_set(audio_queue_array[i], "max-size-bytes", 0, NULL);
        g_object_set(audio_queue_array[i], "max-size-buffers", 3, NULL);

        gst_bin_add(GST_BIN(pipeline->audio_pipeline),audio_queue_array[i]);
        gst_element_sync_state_with_parent(audio_queue_array[i]);

        video_queue_array[i] = gst_element_factory_make ("queue", NULL);
        g_object_set(video_queue_array[i], "max-size-time", 0, NULL);
        g_object_set(video_queue_array[i], "max-size-bytes", 0, NULL);
        g_object_set(video_queue_array[i], "max-size-buffers", 3, NULL);

        gst_bin_add(GST_BIN(pipeline->video_pipeline),video_queue_array[i]);
        gst_element_sync_state_with_parent(video_queue_array[i]);
    }

    pipeline->audio_element[AUDIO_QUEUE_SINK] =             audio_queue_array[0];
    pipeline->audio_element[AUDIO_QUEUE_RESAMPLE] =         audio_queue_array[1];
    pipeline->audio_element[AUDIO_QUEUE_CONVERT] =          audio_queue_array[2];
    pipeline->audio_element[AUDIO_QUEUE_DECODER] =          audio_queue_array[3];

    pipeline->video_element[VIDEO_QUEUE_SINK] =             video_queue_array[5];
    pipeline->video_element[VIDEO_QUEUE_DECODER] =          video_queue_array[7];
}

gpointer
setup_pipeline(RemoteUdp* core)
{
    Pipeline* pipe = remote_app_get_pipeline(core);
    RemoteConfig* config = remote_app_get_qoe(core);
    

    if(pipe->audio_pipeline || pipe->video_pipeline)
        free_pipeline(pipe);

    setup_element_factory(core,
        qoe_get_video_codec(config),
        qoe_get_audio_codec(config));
    setup_pipeline_queue(pipe);
    setup_element_property(core);

    g_signal_connect (pipe->video_element[VIDEO_DECODER], "pad-added",
        G_CALLBACK (handle_video_stream), core);
    g_signal_connect (pipe->audio_element[AUDIO_DECODER], "pad-added",
        G_CALLBACK (handle_audio_stream), core);
    g_signal_connect (pipe->video_element[VIDEO_DECODER], "autoplug-select",
        G_CALLBACK (handle_video_element), core);
    g_signal_connect (pipe->audio_element[AUDIO_DECODER], "autoplug-select",
        G_CALLBACK (handle_audio_element), core);


    if(start_pipeline(pipe->video_pipeline))
        worker_log_output("Starting pipeline");
    else
        worker_log_output("Fail to start pipeline, this may due to pipeline setup failure");

    if(start_pipeline(pipe->audio_pipeline))
        worker_log_output("Starting pipeline");
    else
        worker_log_output("Fail to start pipeline, this may due to pipeline setup failure");
}