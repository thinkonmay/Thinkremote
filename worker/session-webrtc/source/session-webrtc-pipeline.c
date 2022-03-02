/**
 * @file session-webrtc-pipeline.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <session-webrtc-pipeline.h>
#include <session-webrtc-type.h>
#include <session-webrtc-data-channel.h>
#include <session-webrtc-signalling.h>


#include <constant.h>
#include <logging.h>
#include <remote-config.h>
#include <shortcut.h>
#include <handle-key.h>
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

    VIDEO_ELEMENT_LAST
};


/**
 * @brief 
 * 
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

    AUDIO_ELEMENT_LAST
};


struct _Pipeline
{
    /**
     * @brief 
     * GstPipeline of the remote session
     */
	GstElement* pipeline;

    /**
     * @brief 
     * Webrtc bin responsible for creating webrtc connection
     */
	GstElement* webrtcbin;


    GstElement* video_element[VIDEO_ELEMENT_LAST];
    GstElement* audio_element[AUDIO_ELEMENT_LAST];

    GstCaps* video_caps[VIDEO_ELEMENT_LAST];
    GstCaps* audio_caps[AUDIO_ELEMENT_LAST];

    HIDHandler* handler;

    MediaDevice* device;
};





/**
 * @brief 
 * get and set the visibility of pointer
 * @param capture 
 */
static void
toggle_pointer_on(GstElement* capture)
{
    g_object_set(capture, "show-cursor", TRUE, NULL); 
}

/**
 * @brief 
 * get and set the visibility of pointer
 * @param capture 
 */
static void
toggle_pointer_off(GstElement* capture)
{
    g_object_set(capture, "show-cursor", FALSE, NULL); 
}

Pipeline*
pipeline_initialize()
{
    Pipeline* pipeline = malloc(sizeof(Pipeline));
    memset(pipeline,0,sizeof(Pipeline));
    pipeline->device = get_media_device_source();
    return pipeline;
}

void
free_pipeline(Pipeline* pipeline)
{
    deactivate_hid_handler(pipeline->handler);
    gst_element_set_state (pipeline->pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline->pipeline);
    memset(pipeline,0,sizeof(Pipeline));
}

#define MAXIMUM_VIDEO_BITRATE       4194303
#define MAXIMUM_AUDIO_BITRATE       650000

#define MINIMUM_VIDEO_BITRATE       1
#define MINIMUM_AUDIO_BITRATE       4000

#define VIDEO_BITRATE_STEP          200
#define AUDIO_BITRATE_STEP          200000

static void
increase_stream_bitrate(gpointer data)
{
    Pipeline* pipeline = (Pipeline*) data;
    GstElement* audio_encoder = pipeline->audio_element[SOUND_ENCODER];
    GstElement* video_encoder = pipeline->video_element[VIDEO_ENCODER];

    gint audio_bitrate; 
    gint video_bitrate;
    g_object_get(audio_encoder,"bitrate",&audio_bitrate,NULL);
    g_object_get(video_encoder,"bitrate",&video_bitrate,NULL);

    audio_bitrate = audio_bitrate + AUDIO_BITRATE_STEP;
    video_bitrate = video_bitrate + VIDEO_BITRATE_STEP;

    if((audio_bitrate < MINIMUM_AUDIO_BITRATE) || 
       (video_bitrate < MINIMUM_VIDEO_BITRATE))
       return;

    g_object_set(audio_encoder,"bitrate",audio_bitrate,NULL);
    g_object_set(video_encoder,"bitrate",video_bitrate,NULL);
}

static void
decrease_stream_bitrate(gpointer data)
{
    Pipeline* pipeline = (Pipeline*) data;
    GstElement* audio_encoder = pipeline->audio_element[SOUND_ENCODER];
    GstElement* video_encoder = pipeline->video_element[VIDEO_ENCODER];

    gint audio_bitrate;
    gint video_bitrate;
    g_object_get(audio_encoder,"bitrate",&audio_bitrate,NULL);
    g_object_get(video_encoder,"bitrate",&video_bitrate,NULL);

    audio_bitrate = audio_bitrate - AUDIO_BITRATE_STEP;
    video_bitrate = video_bitrate - VIDEO_BITRATE_STEP;

    if((audio_bitrate < MINIMUM_AUDIO_BITRATE) || 
       (video_bitrate < MINIMUM_VIDEO_BITRATE))
       return;

    g_object_set(audio_encoder,"bitrate",audio_bitrate,NULL);
    g_object_set(video_encoder,"bitrate",video_bitrate,NULL);

}

static gboolean
start_pipeline(SessionCore* core)
{
    GstStateChangeReturn ret;
    Pipeline* pipe = session_core_get_pipeline(core);
    worker_log_output("Starting pipeline");

    ret = GST_IS_ELEMENT(pipe->pipeline);    
    ret = gst_element_set_state(GST_ELEMENT(pipe->pipeline), GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        worker_log_output("Fail to start pipeline, this may due to pipeline setup failure");
        session_core_finalize(core, NULL);
    }

    Shortcut* shortcuts = shortcut_list_initialize(10);

    add_new_shortcut_to_list(shortcuts,NULL,
            WORKER_POINTER_ON,(ShortcutHandleFunction)toggle_pointer_on,
            pipe->video_element[SCREEN_CAPTURE]);

    add_new_shortcut_to_list(shortcuts,NULL,
            WORKER_POINTER_OFF,(ShortcutHandleFunction)toggle_pointer_off,
            pipe->video_element[SCREEN_CAPTURE]);

    add_new_shortcut_to_list(shortcuts,NULL,
            DECREASE_STREAM_BITRATE,(ShortcutHandleFunction)decrease_stream_bitrate,
            pipe);

    add_new_shortcut_to_list(shortcuts,NULL,
            INCREASE_STREAM_BITRATE,(ShortcutHandleFunction)increase_stream_bitrate,
            pipe);

    pipe->handler = activate_hid_handler(pipe->video_element[SCREEN_CAPTURE],shortcuts);
	shortcut_list_free(shortcuts);

	start_qos_thread(core);
    return TRUE;
}

#ifdef G_OS_WIN32
#define DIRECTX_PAD "video/x-raw(memory:D3D11Memory)"
#endif

#define RTP_CAPS_AUDIO "application/x-rtp,media=audio,payload=96,encoding-name="
#define RTP_CAPS_VIDEO "application/x-rtp,media=video,payload=97,encoding-name="
#define QUEUE "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 


GstElement*
backup_software_decoder(Codec codec)
{
    GError *error = NULL;
#ifdef G_OS_WIN32
    return gst_parse_launch("webrtcbin bundle-policy=max-bundle name=sendrecv "

            "d3d11screencapturesrc name=screencap ! "
            DIRECTX_PAD",framerate=60/1 ! "                             QUEUE
            "d3d11convert ! "DIRECTX_PAD",format=NV12 ! "               QUEUE
            "d3d11download ! "                                          QUEUE
            "mfh264enc ! "                                              QUEUE
            "rtph264pay name=rtp ! "                                    QUEUE
            RTP_CAPS_VIDEO "H264 ! sendrecv. "

            "wasapi2src loopback=true name=audiocapsrc !"               QUEUE 
            "audioconvert ! "                                           QUEUE 
            "audioresample ! "                                          QUEUE 
            "opusenc name=audioencoder ! "                              QUEUE 
            "rtpopuspay ! "                                             QUEUE 
            RTP_CAPS_AUDIO "OPUS ! sendrecv. ", &error);
#endif
}

static void
setup_element_factory(SessionCore* core,
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
        pipe->pipeline =
            gst_parse_launch("webrtcbin bundle-policy=max-bundle name=sendrecv "

                "d3d11screencapturesrc name=screencap ! "
                DIRECTX_PAD",framerate=60/1 ! "                            QUEUE
                "d3d11convert ! "DIRECTX_PAD",format=NV12 ! "              QUEUE
                "mfh264enc name=videoencoder ! "                           QUEUE
                "rtph264pay name=rtp ! "                                   QUEUE
                RTP_CAPS_VIDEO "H264 ! sendrecv. "

                "wasapi2src loopback=true name=audiocapsrc !"              QUEUE
                "audioconvert ! "                                          QUEUE 
                "audioresample ! "                                         QUEUE 
                "opusenc name=audioencoder ! "                             QUEUE 
                "rtpopuspay ! "                                            QUEUE 
                RTP_CAPS_AUDIO "OPUS ! sendrecv. ", &error);
#else
        pipe->pipeline =
            gst_parse_launch("webrtcbin bundle-policy=max-bundle name=sendrecv "

                "ximagesrc name=screencap ! "                              QUEUE
                "videoconvert ! "                                          QUEUE
                "x264enc name=videoencoder ! "                             QUEUE
                "rtph264pay name=rtp ! "                                   QUEUE
                RTP_CAPS_VIDEO "H264 ! sendrecv. "

                "pulsesrc name=audiocapsrc !"                              QUEUE
                "audioconvert ! "                                          QUEUE 
                "audioresample ! "                                         QUEUE 
                "opusenc name=audioencoder ! "                             QUEUE 
                "rtpopuspay ! "                                            QUEUE 
                RTP_CAPS_AUDIO "OPUS ! sendrecv. ", &error);

#endif
    }
    else if (video == CODEC_H265)
    {
#ifdef G_OS_WIN32
        pipe->pipeline =
            gst_parse_launch("webrtcbin bundle-policy=max-bundle name=sendrecv "

                "d3d11screencapturesrc name=screencap ! "
                DIRECTX_PAD",framerate=60/1 ! "                             QUEUE
                "d3d11convert ! "DIRECTX_PAD",format=NV12 ! "               QUEUE
                "mfh265enc name=videoencoder ! "                            QUEUE
                "rtph265pay name=rtp ! "                                    QUEUE 
                RTP_CAPS_VIDEO "H265 ! sendrecv. "

                "wasapi2src loopback=true name=audiocapsrc !"               QUEUE 
                "audioconvert ! "                                           QUEUE 
                "audioresample ! "                                          QUEUE 
                "opusenc name=audioencoder ! "                              QUEUE 
                "rtpopuspay ! "                                             QUEUE 
                RTP_CAPS_AUDIO "OPUS ! sendrecv. ", &error);
#else
        pipe->pipeline =
            gst_parse_launch("webrtcbin bundle-policy=max-bundle name=sendrecv "

                "ximagesrc name=screencap ! "                               QUEUE
                "videoconvert name=videoencoder ! "                         QUEUE
                "x265enc name=videoencoder ! "                              QUEUE
                "rtph265pay name=rtp ! "                                    QUEUE 
                RTP_CAPS_VIDEO "H265 ! sendrecv. "

                "pulsesrc loopback=true name=audiocapsrc ! "                QUEUE 
                "audioconvert ! "                                           QUEUE 
                "audioresample ! "                                          QUEUE 
                "opusenc name=audioencoder ! "                              QUEUE 
                "rtpopuspay ! "                                             QUEUE 
                RTP_CAPS_AUDIO"OPUS ! sendrecv. ", &error);

#endif
    }

    if (error || !pipe->pipeline) 
    { 
        if(g_str_has_prefix(error->message,"no element"));
        {
            pipe->pipeline = backup_software_decoder(video);
        }
    }

    if(!pipe->pipeline)
        session_core_finalize(core,error);

    pipe->audio_element[SOUND_SOURCE] = 
        gst_bin_get_by_name(GST_BIN(pipe->pipeline), "audiocapsrc");
    pipe->video_element[VIDEO_ENCODER] = 
        gst_bin_get_by_name(GST_BIN(pipe->pipeline), "videoencoder");
    pipe->video_element[RTP_VIDEO_PAYLOAD] = 
        gst_bin_get_by_name(GST_BIN(pipe->pipeline), "rtp");
    pipe->video_element[SCREEN_CAPTURE] = 
        gst_bin_get_by_name(GST_BIN(pipe->pipeline), "screencap");
    pipe->audio_element[SOUND_ENCODER] = 
        gst_bin_get_by_name(GST_BIN(pipe->pipeline), "audioencoder");
    pipe->webrtcbin =
        gst_bin_get_by_name(GST_BIN(pipe->pipeline), "sendrecv");
}












/**
 * @brief Set the up element property object
 * setup proerty of gst element,
 * this function should be called after pipeline factory has been done,
 * each element are assigned to an element in pipeline
 * @param core 
 */
static void
setup_element_property(SessionCore* core)
{
    Pipeline* pipe = session_core_get_pipeline(core);
    SignallingHub* hub = session_core_get_signalling_hub(core);
    StreamConfig* qoe = session_core_get_qoe(core);

#ifdef G_OS_WIN32
    if (pipe->video_element[SCREEN_CAPTURE]) { g_object_set(pipe->video_element[SCREEN_CAPTURE], "show-cursor", FALSE, NULL);}
#else
    if (pipe->video_element[SCREEN_CAPTURE]) { g_object_set(pipe->video_element[SCREEN_CAPTURE], "show-pointer", TRUE, NULL);}

    if (pipe->video_element[SCREEN_CAPTURE]) { g_object_set(pipe->video_element[SCREEN_CAPTURE], "remote", TRUE, NULL);}

    if (pipe->video_element[SCREEN_CAPTURE]) { g_object_set(pipe->video_element[SCREEN_CAPTURE], "blocksize", 16384, NULL);}
    
    if (pipe->video_element[SCREEN_CAPTURE]) { g_object_set(pipe->video_element[SCREEN_CAPTURE], "use-damage", FALSE, NULL);}
#endif

#ifdef G_OS_WIN32
    if (pipe->video_element[VIDEO_ENCODER]) { g_object_set(pipe->video_element[VIDEO_ENCODER], "rc-mode", 0, NULL); }

    if (pipe->video_element[VIDEO_ENCODER]) { g_object_set(pipe->video_element[VIDEO_ENCODER], "quality-vs-speed", 100, NULL); }

    if (pipe->video_element[VIDEO_ENCODER]) { g_object_set(pipe->video_element[VIDEO_ENCODER], "bitrate", qoe_get_video_bitrate(qoe), NULL); }

    if (pipe->video_element[VIDEO_ENCODER]) { g_object_set(pipe->video_element[VIDEO_ENCODER], "low-latency", TRUE, NULL); }
#else
    if (pipe->video_element[VIDEO_ENCODER]) { g_object_set(pipe->video_element[VIDEO_ENCODER], "threads", 4, NULL);}

    if (pipe->video_element[VIDEO_ENCODER]) { g_object_set(pipe->video_element[VIDEO_ENCODER], "bframes", 0, NULL);}

    if (pipe->video_element[VIDEO_ENCODER]) { g_object_set(pipe->video_element[VIDEO_ENCODER], "key-int-max", 0, NULL);}

    if (pipe->video_element[VIDEO_ENCODER]) { g_object_set(pipe->video_element[VIDEO_ENCODER], "byte-stream", TRUE, NULL);}

    if (pipe->video_element[VIDEO_ENCODER]) { g_object_set(pipe->video_element[VIDEO_ENCODER], "tune", "zerolatency", NULL);}

    if (pipe->video_element[VIDEO_ENCODER]) { g_object_set(pipe->video_element[VIDEO_ENCODER], "speed-preset", "veryfast", NULL);}

    if (pipe->video_element[VIDEO_ENCODER]) { g_object_set(pipe->video_element[VIDEO_ENCODER], "pass", "pass1", NULL);}

    if (pipe->video_element[VIDEO_ENCODER]) { g_object_set(pipe->video_element[VIDEO_ENCODER], "bitrate", qoe_get_video_bitrate(qoe), NULL); }
#endif

    if (pipe->video_element[RTP_VIDEO_PAYLOAD]) { g_object_set(pipe->video_element[RTP_VIDEO_PAYLOAD], "aggregate-mode", 1, NULL);}

#ifndef G_OS_WIN32
    if (pipe->audio_element[SOUND_SOURCE]) { g_object_set(pipe->audio_element[SOUND_SOURCE], "provide-clock", TRUE, NULL);}

    if (pipe->audio_element[SOUND_SOURCE]) { g_object_set(pipe->audio_element[SOUND_SOURCE], "do-timestamp", TRUE, NULL);}
#else
    if (pipe->audio_element[SOUND_SOURCE]) { g_object_set(pipe->audio_element[SOUND_SOURCE], "low-latency", TRUE, NULL);}
#endif

    if (pipe->audio_element[SOUND_SOURCE]) { g_object_set(pipe->audio_element[SOUND_SOURCE], "device", get_audio_source(pipe->device), NULL); }

    if (pipe->video_element[SCREEN_CAPTURE]) { g_object_set(pipe->video_element[SCREEN_CAPTURE], "monitor-handle", get_video_source(pipe->device), NULL); }

    g_object_set(pipe->webrtcbin,"latency",0,NULL);
}






void
setup_pipeline(SessionCore* core)
{
    SignallingHub* signalling = session_core_get_signalling_hub(core);
    Pipeline* pipe = session_core_get_pipeline(core);
    StreamConfig* qoe = session_core_get_qoe(core);

    if(pipe->pipeline)
        free_pipeline(pipe);
    
    setup_element_factory(core, 
        qoe_get_video_codec(qoe),
        qoe_get_audio_codec(qoe));

    signalling_hub_setup_turn_and_stun(pipe,signalling);
    connect_signalling_handler(core);
    setup_element_property(core);

    #ifdef DEFAULT_TURN
    g_object_set(pipe->webrtcbin,"ice-transport-policy",1,NULL);
    #endif


    GstStateChangeReturn result = gst_element_change_state(pipe->pipeline, GST_STATE_READY);
    if (result == GST_STATE_CHANGE_FAILURE)
    {
        worker_log_output("Fail to start pipeline, this may due to pipeline setup failure");
        session_core_finalize(core, NULL);
    }
    connect_data_channel_signals(core);
    start_pipeline(core);
}






GstElement*
pipeline_get_webrtc_bin(Pipeline* pipe)
{
    return pipe->webrtcbin;
}