/**
 * @file session-udp-pipeline.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <session-udp-pipeline.h>
#include <session-udp-type.h>
#include <session-udp.h>


#include <remote-config.h>
#include <logging.h>
#include <enum.h>
#include <device.h>

#include <gst/gst.h>
#include <glib-2.0/glib.h>
#include <gst/webrtc/webrtc.h>







/**
 * @brief 
 * gstreamer video element enumaration,
 * the order of element in enum must follow the 
 */
enum
{
    /*screen capture source*/
    SCREEN_CAPTURE,

    /*video encoder*/
    VIDEO_ENCODER,

    /*payload packetize*/
    RTP_VIDEO_PAYLOAD,

    UDP_VIDEO_SINK,

    VIDEO_ELEMENT_LAST
};

/**
 * @brief 
 * gstreamer audio element enumaration,
 * the order of element in enum must follow the 
 */
enum
{
    /*audio capture source*/
    SOUND_SOURCE,

    /*audio encoder*/
    SOUND_ENCODER,

    /*rtp packetize and queue*/
    RTP_AUDIO_PAYLOAD,

    UDP_AUDIO_SINK,

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

    MediaDevice* device;
};




Pipeline*
pipeline_initialize()
{
    Pipeline* pipeline = malloc(sizeof(Pipeline));
    memset(pipeline,0,sizeof(Pipeline));
    return pipeline;
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

#ifdef G_OS_WIN32
#define DIRECTX_PAD "video/x-raw(memory:D3D11Memory)"
#endif

#define RTP_CAPS_AUDIO "application/x-rtp,media=audio,payload=96,encoding-name="
#define RTP_CAPS_VIDEO "application/x-rtp,media=video,payload=97,encoding-name="
#define QUEUE "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 



static void
setup_element_factory(SessionUdp* core,
                      Codec video, 
                      Codec audio)
{
    Pipeline* pipe = session_core_get_pipeline(core);
    GError* error = NULL;

    if (audio != OPUS_ENC) 
        return;
    
    if (video == CODEC_H264)
    {
#ifdef G_OS_WIN32
        pipe->video_pipeline =
            gst_parse_launch(
                "d3d11screencapturesrc name=screencap ! "                     QUEUE
                DIRECTX_PAD",framerate=60/1 ! "                            QUEUE
                "d3d11convert ! "DIRECTX_PAD",format=NV12 ! "              QUEUE
                "mfh264enc name=videoencoder ! "                           QUEUE
                "rtph264pay name=rtp ! "                                   QUEUE
                RTP_CAPS_VIDEO "H264 ! "                                   QUEUE
                "udpsink name=udp", &error);

        pipe->audio_pipeline = 
            gst_parse_launch(
                "wasapi2src name=audiocapsrc !"                            QUEUE
                "audioconvert ! "                                          QUEUE 
                "audioresample ! "                                         QUEUE 
                "opusenc name=audioencoder ! "                             QUEUE 
                "rtpopuspay ! "                                            QUEUE 
                RTP_CAPS_AUDIO "OPUS ! "                                   QUEUE
                "udpsink name=udp", &error);
#else
        pipe->video_pipeline =
            gst_parse_launch(
                "ximagesrc name=screencap ! "                              QUEUE
                "videoconvert ! "                                          QUEUE
                "x264enc name=videoencoder ! "                             QUEUE
                "rtph264pay name=rtp ! "                                   QUEUE
                RTP_CAPS_VIDEO "H264 ! sendrecv. "
                "udpsink name=udp", &error);

        pipe->audio_pipeline = 
            gst_parse_launch(
                "pulsesrc name=audiocapsrc !"                              QUEUE
                "audioconvert ! "                                          QUEUE 
                "audioresample ! "                                         QUEUE 
                "opusenc name=audioencoder ! "                             QUEUE 
                "rtpopuspay ! "                                            QUEUE 
                RTP_CAPS_AUDIO "OPUS ! "
                "udpsink name=udp", &error);
#endif
    }
    else if (video == CODEC_H265)
    {
#ifdef G_OS_WIN32
        pipe->video_pipeline =
            gst_parse_launch(
                "d3d11screencapturesrc name=screencap ! "                     QUEUE
                DIRECTX_PAD",framerate=60/1 ! "                            QUEUE
                "d3d11convert ! "DIRECTX_PAD",format=NV12 ! "              QUEUE
                "mfh265enc name=videoencoder ! "                           QUEUE
                "rtph265pay name=rtp ! "                                   QUEUE
                RTP_CAPS_VIDEO "H265 ! "                                   QUEUE
                "udpsink name=udp", &error);

        pipe->audio_pipeline = 
            gst_parse_launch(
                "wasapi2src name=audiocapsrc !"                            QUEUE
                "audioconvert ! "                                          QUEUE 
                "audioresample ! "                                         QUEUE 
                "opusenc name=audioencoder ! "                             QUEUE 
                "rtpopuspay ! "                                            QUEUE 
                RTP_CAPS_AUDIO "OPUS ! "                                   QUEUE
                "udpsink name=udp", &error);
#else
        pipe->video_pipeline =
            gst_parse_launch(
                "d3d11screencapturesrc name=screencap ! "                     QUEUE
                "d3d11convert ! "                                          QUEUE
                "mfh265enc name=videoencoder ! "                           QUEUE
                "rtph265pay name=rtp ! "                                   QUEUE
                RTP_CAPS_VIDEO "H265 ! "                                   QUEUE
                "udpsink name=udp", &error);

        pipe->audio_pipeline = 
            gst_parse_launch(
                "wasapi2src name=audiocapsrc !"                            QUEUE
                "audioconvert ! "                                          QUEUE 
                "audioresample ! "                                         QUEUE 
                "opusenc name=audioencoder ! "                             QUEUE 
                "rtpopuspay ! "                                            QUEUE 
                RTP_CAPS_AUDIO "OPUS ! "                                   QUEUE
                "udpsink name=udp", &error);
#endif
    }


    if (error) 
        session_core_finalize(core,error);

    pipe->audio_element[SOUND_SOURCE] = 
        gst_bin_get_by_name(GST_BIN(pipe->audio_pipeline), "audiocapsrc");
    pipe->audio_element[SOUND_ENCODER] = 
        gst_bin_get_by_name(GST_BIN(pipe->audio_pipeline), "audioencoder");
    pipe->audio_element[UDP_AUDIO_SINK] = 
        gst_bin_get_by_name(GST_BIN(pipe->audio_pipeline), "udp");

    pipe->video_element[VIDEO_ENCODER] = 
        gst_bin_get_by_name(GST_BIN(pipe->video_pipeline), "videoencoder");
    pipe->video_element[RTP_VIDEO_PAYLOAD] = 
        gst_bin_get_by_name(GST_BIN(pipe->video_pipeline), "rtp");
    pipe->video_element[SCREEN_CAPTURE] = 
        gst_bin_get_by_name(GST_BIN(pipe->video_pipeline), "screencap");
    pipe->video_element[UDP_VIDEO_SINK] = 
        gst_bin_get_by_name(GST_BIN(pipe->video_pipeline), "udp");
}







/**
 * @brief Set the up element property object
 * setup proerty of gst element,
 * this function should be called after pipeline factory has been done,
 * each element are assigned to an element in pipeline
 * @param core 
 */
static void
setup_element_property(SessionUdp* core)
{
    Pipeline* pipe = session_core_get_pipeline(core);
    SignallingHub* hub = session_core_get_signalling_hub(core);
    StreamConfig* qoe = session_core_get_qoe(core);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef G_OS_WIN32
    g_object_set(pipe->audio_element[SOUND_SOURCE], "provide-clock", TRUE, NULL);

    g_object_set(pipe->audio_element[SOUND_SOURCE], "do-timestamp", TRUE, NULL);
#else
    g_object_set(pipe->audio_element[SOUND_SOURCE], "low-latency", TRUE, NULL);

    g_object_set(pipe->audio_element[SOUND_SOURCE], "loopback", TRUE, NULL);
#endif
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef G_OS_WIN32
    g_object_set(pipe->video_element[SCREEN_CAPTURE], "show-cursor", FALSE, NULL);
#else
    g_object_set(pipe->video_element[SCREEN_CAPTURE], "show-pointer", TRUE, NULL);

    g_object_set(pipe->video_element[SCREEN_CAPTURE], "remote", TRUE, NULL);

    g_object_set(pipe->video_element[SCREEN_CAPTURE], "blocksize", 16384, NULL);
    
    g_object_set(pipe->video_element[SCREEN_CAPTURE], "use-damage", FALSE, NULL);
#endif
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef G_OS_WIN32
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    g_object_set(pipe->video_element[VIDEO_ENCODER], "rc-mode", 0, NULL); 

    g_object_set(pipe->video_element[VIDEO_ENCODER], "quality-vs-speed", 100, NULL); 

    g_object_set(pipe->video_element[VIDEO_ENCODER], "bitrate", qoe_get_video_bitrate(qoe), NULL); 

    g_object_set(pipe->video_element[VIDEO_ENCODER], "low-latency", TRUE, NULL); 
#else
    g_object_set(pipe->video_element[VIDEO_ENCODER], "threads", 4, NULL);

    g_object_set(pipe->video_element[VIDEO_ENCODER], "bframes", 0, NULL);

    g_object_set(pipe->video_element[VIDEO_ENCODER], "key-int-max", 0, NULL);

    g_object_set(pipe->video_element[VIDEO_ENCODER], "byte-stream", TRUE, NULL);

    g_object_set(pipe->video_element[VIDEO_ENCODER], "tune", "zerolatency", NULL);

    g_object_set(pipe->video_element[VIDEO_ENCODER], "speed-preset", "veryfast", NULL);

    g_object_set(pipe->video_element[VIDEO_ENCODER], "pass", "pass1", NULL);

    g_object_set(pipe->video_element[VIDEO_ENCODER], "bitrate", qoe_get_video_bitrate(qoe), NULL); 
#endif
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (pipe->video_element[RTP_VIDEO_PAYLOAD]) { g_object_set(pipe->video_element[RTP_VIDEO_PAYLOAD], "aggregate-mode", 1, NULL);}
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (pipe->audio_element[SOUND_SOURCE]) { g_object_set(pipe->audio_element[SOUND_SOURCE], "device", get_audio_source(pipe->device), NULL); }

    if (pipe->video_element[SCREEN_CAPTURE]) { g_object_set(pipe->video_element[SCREEN_CAPTURE], "monitor-handle", get_video_source(pipe->device), NULL); }
}



void
setup_udp_endpoint(Pipeline* pipeline,
                   SessionUdp* udp)
{
    UdpEndpoint* audio = session_core_get_audio_endpoint(udp);
    UdpEndpoint* video = session_core_get_video_endpoint(udp);
    setup_udp_endpoint(pipeline->audio_element[UDP_AUDIO_SINK],audio);
    setup_udp_endpoint(pipeline->video_element[UDP_VIDEO_SINK],video);
}



void
setup_pipeline(SessionUdp* core)
{
    SignallingHub* signalling = session_core_get_signalling_hub(core);
    Pipeline* pipe = session_core_get_pipeline(core);
    StreamConfig* qoe = session_core_get_qoe(core);

    if(pipe->audio_pipeline || pipe->video_pipeline)
        free_pipeline(pipe);
    
    setup_element_factory(core, 
        qoe_get_video_codec(qoe),
        qoe_get_audio_codec(qoe));

    setup_element_property(core);
    setup_udp_endpoint(pipe,core);

    if(start_pipeline(pipe->video_pipeline))
        worker_log_output("Starting pipeline");
    else
        worker_log_output("Fail to start pipeline, this may due to pipeline setup failure");

    if(start_pipeline(pipe->audio_pipeline))
        worker_log_output("Starting pipeline");
    else
        worker_log_output("Fail to start pipeline, this may due to pipeline setup failure");
}



