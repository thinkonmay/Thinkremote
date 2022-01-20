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
#include <remote-udp-gui.h>
#include <remote-udp-input.h>

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

    /**
     * @brief 
     * 
     */
    VIDEO_DECODER,

    /**
     * @brief 
     * 
     */
    VIDEO_CODEC_PARSER,

    /**
     * @brief 
     * 
     */
    VIDEO_DEPAYLOAD,

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
    /**
     * @brief 
     * 
     */
    AUDIO_RESAMPLE,
    /**
     * @brief 
     * 
     */
    AUDIO_CONVERT,
    /**
     * @brief 
     * 
     */
    AUDIO_DECODER,
    /**
     * @brief 
     * 
     */
    AUDIO_DEPAYLOAD,


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
setup_video_sink_navigator(RemoteUdp* core)
{
    Pipeline* pipeline = remote_app_get_pipeline(core);
    GstPad* pad = gst_element_get_static_pad(pipeline->video_element[VIDEO_CONVERT],"src");

    gst_pad_set_event_function_full(pad,handle_event,core,NULL);
}
#endif

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
#ifdef G_OS_WIN32
            pipe->video_pipeline =
                gst_parse_launch(
                    "udpsrc name=udp ! "                                       
                    "application/x-rtp,media=video,encoding-name=H264"         QUEUE
                    "rtph264depay ! "                                          QUEUE
                    "h264parse ! "                                             QUEUE
                    "d3d11h264dec name=videoencoder ! "                        QUEUE
                    "d3d11videosink name=sink", &error);
#else
            pipe->video_pipeline =
                gst_parse_launch(
                    "udpsrc name=udp ! "                                       QUEUE
                    "rtph264depay ! "                                          QUEUE
                    "h264parse ! "                                             QUEUE
                    "avdec_h264 name=videoencoder ! "                          QUEUE
                    "autovideosink name=sink", &error);
#endif

            pipe->audio_pipeline = 
                gst_parse_launch(
                    "udpsrc name=udp !"                                        QUEUE
                    "rtpopusdepay ! "                                          QUEUE 
                    "opusparse ! "                                             QUEUE 
                    "opusdec name=audioencoder ! "                             QUEUE 
                    "audioconvert ! "                                          QUEUE 
                    "audioresample ! "                                         QUEUE 
                    "autoaudiosink ", &error);
        }
    }
    else if (video == CODEC_H265)
    {
        if (audio == OPUS_ENC)
        {
#ifdef G_OS_WIN32
            pipe->video_pipeline =
                gst_parse_launch(
                    "udpsrc name=udp ! "                                       QUEUE
                    "rtph265depay ! "                                          QUEUE
                    "h265parse ! "                                             QUEUE
                    "d3d11h265dec ! "                                          QUEUE
                    "d3d11videosink name=sink", &error);
#else
            pipe->video_pipeline =
                gst_parse_launch(
                    "udpsrc name=udp ! "
                    "application/x-rtp, media=video, encoding-name=H265"       QUEUE
                    "rtph265depay ! "                                          QUEUE
                    "h265parse ! "                                             QUEUE
                    "avdec_h265 name=videoencoder ! "                          QUEUE
                    "autovideosink name=sink", &error);

#endif
            pipe->audio_pipeline = 
                gst_parse_launch(
                    "udpsrc name=udp !"                                        QUEUE
                    "rtpopusdepay ! "                                          QUEUE 
                    "opusparse ! "                                             QUEUE 
                    "opusdec name=audioencoder ! "                             QUEUE 
                    "audioconvert ! "                                          QUEUE 
                    "audioresample ! "                                         QUEUE 
                    "autoaudiosink ", &error);

        }
    }


    if (error) { return; }

    pipe->audio_element[UDP_AUDIO_SOURCE] = 
        gst_bin_get_by_name(GST_BIN(pipe->audio_pipeline), "udp");

    pipe->video_element[UDP_VIDEO_SOURCE] = 
        gst_bin_get_by_name(GST_BIN(pipe->video_pipeline), "udp");
    pipe->video_element[VIDEO_SINK] = 
        gst_bin_get_by_name(GST_BIN(pipe->video_pipeline), "sink");

    
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
    g_object_set(pipe->video_element[VIDEO_ENCODER], "threads", 4, NULL);

    g_object_set(pipe->video_element[VIDEO_ENCODER], "bframes", 0, NULL);

    g_object_set(pipe->video_element[VIDEO_ENCODER], "key-int-max", 0, NULL);

    g_object_set(pipe->video_element[VIDEO_ENCODER], "byte-stream", TRUE, NULL);

    g_object_set(pipe->video_element[VIDEO_ENCODER], "tune", "zerolatency", NULL);

    g_object_set(pipe->video_element[VIDEO_ENCODER], "speed-preset", "veryfast", NULL);

    g_object_set(pipe->video_element[VIDEO_ENCODER], "pass", "pass1", NULL);
#endif
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    g_object_set(pipe->video_element[UDP_VIDEO_SOURCE], "port", pipe->video_port, NULL);

    g_object_set(pipe->audio_element[UDP_AUDIO_SOURCE], "port", pipe->audio_port, NULL);
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

    setup_element_property(core);

    setup_video_overlay(
        pipe->video_element[VIDEO_SINK],
        pipe->video_pipeline,core);

    if(start_pipeline(pipe->video_pipeline))
        worker_log_output("Starting pipeline");
    else
        worker_log_output("Fail to start pipeline, this may due to pipeline setup failure");

    if(start_pipeline(pipe->audio_pipeline))
        worker_log_output("Starting pipeline");
    else
        worker_log_output("Fail to start pipeline, this may due to pipeline setup failure");

}








