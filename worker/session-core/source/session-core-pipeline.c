/**
 * @file session-core-pipeline.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <session-core-pipeline.h>
#include <session-core-type.h>
#include <session-core-data-channel.h>
#include <session-core-signalling.h>
#include <session-core-remote-config.h>


#include <logging.h>
#include <qoe.h>

#include <gst/gst.h>
#include <glib-2.0/glib.h>
#include <gst/webrtc\webrtc.h>
#include <gst/rtp\gstrtppayloads.h>






/// <summary>
/// gstreamer video element enumaration,
/// the order of element in enum must follow the 
/// </summary>
enum
{
    /*screen capture source*/
    DIRECTX_SCREEN_CAPTURE_SOURCE,

    /*preprocess before encoding*/
    CUDA_UPLOAD,
    CUDA_CONVERT,
    VIDEO_CONVERT,
    /*video encoder*/

    NVIDIA_H264_ENCODER,
    NVIDIA_H265_ENCODER,
    H264_MEDIA_FOUNDATION,
    H265_MEDIA_FOUNDATION,

    VP9_ENCODER,
    VP8_ENCODER,


    /*payload packetize*/
    RTP_H264_PAYLOAD,
    RTP_H265_PAYLOAD,
    RTP_VP9_PAYLOAD,
    RTP_VP8_PAYLOAD,

    VIDEO_ELEMENT_LAST
};

/// <summary>
/// gstreamer audio element enumaration,
/// the order of element in enum must follow the 
/// </summary>
enum
{
    /*audio capture source*/
    SOUND_SOURCE,

    /*audio encoder*/
    SOUND_ENCODER,

    /*rtp packetize and queue*/
    RTP_OPUS_PAYLOAD,
    RTP_RTX_QUEUE,

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
};

static gchar sound_capture_device_id[1000]  = {0};
static gchar sound_output_device_id[1000]   = {0};


void device_foreach(gpointer data, gpointer user_data);

Pipeline*
pipeline_initialize(SessionCore* core)
{
    static Pipeline pipeline;
    memset(&pipeline,0,sizeof(pipeline));

    GstDeviceMonitor* monitor = gst_device_monitor_new();
    if(!gst_device_monitor_start(monitor)) {
        worker_log_output("WARNING: Monitor couldn't started!!\n");
    }

    worker_log_output("Searching for available device");
    GList* device_list = gst_device_monitor_get_devices(monitor);
    g_list_foreach(device_list,(GFunc)device_foreach,NULL);

    return &pipeline;
}

static gboolean
start_pipeline(SessionCore* core)
{
    GstStateChangeReturn ret;
    Pipeline* pipe = session_core_get_pipeline(core);

    ret = GST_IS_ELEMENT(pipe->pipeline);    

    ret = gst_element_set_state(GST_ELEMENT(pipe->pipeline), GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        GError error;
        error.message = "Fail to start pipeline, this may due to pipeline setup failure";
        session_core_finalize(core, &error);
    }
    worker_log_output("Starting pipeline");
	start_qos_thread(core);
    return TRUE;
}


#define DIRECTX_PAD "video/x-raw(memory:D3D11Memory)"
#define RTP_CAPS_OPUS "application/x-rtp,media=audio,payload=96,encoding-name="
#define RTP_CAPS_VIDEO "application/x-rtp,media=video,payload=97,encoding-name="




static void
setup_element_factory(SessionCore* core,
                      Codec video, 
                      Codec audio)
{
    Pipeline* pipe = session_core_get_pipeline(core);
    GError* error = NULL;
    
    if (video == CODEC_H264)
    {
        if (audio == OPUS_ENC) 
        {
            // setup default nvenc encoder (nvidia encoder)
            pipe->pipeline =
                gst_parse_launch("webrtcbin bundle-policy=max-bundle name=sendrecv "

                    "d3d11desktopdupsrc name=screencap ! "DIRECTX_PAD",framerate=60/1 ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! "
                    "d3d11convert ! "DIRECTX_PAD",format=NV12 ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! "
                    "mfh264enc name=videoencoder ! video/x-h264,profile=high ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! "
                    "rtph264pay name=rtp ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    RTP_CAPS_VIDEO "H264 ! sendrecv. "

                    "wasapi2src loopback=true name=audiocapsrc name=audiocapsrc !" 
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    "audioconvert ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    "audioresample ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    "opusenc name=audioencoder ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    "rtpopuspay ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    RTP_CAPS_OPUS "OPUS ! " "sendrecv. ", &error);

            pipe->audio_element[SOUND_SOURCE] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "audiocapsrc");
            pipe->video_element[H264_MEDIA_FOUNDATION] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "videoencoder");
            pipe->video_element[RTP_H264_PAYLOAD] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "rtp");
            pipe->video_element[DIRECTX_SCREEN_CAPTURE_SOURCE] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "screencap");
            pipe->audio_element[SOUND_ENCODER] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "audioencoder");
        }
    }
    else if (video == CODEC_H265)
    {
        if (audio == OPUS_ENC)
        {
            // setup default nvenc encoder (nvidia encoder)
            pipe->pipeline =
                gst_parse_launch("webrtcbin bundle-policy=max-bundle name=sendrecv "
                    "d3d11desktopdupsrc name=screencap ! "DIRECTX_PAD",framerate=60/1 ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! "
                    "d3d11convert ! "DIRECTX_PAD",format=NV12 ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! "
                    "mfh265enc name=videoencoder ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! "
                    "rtph265pay name=rtp ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    RTP_CAPS_VIDEO "H265 ! sendrecv. "

                    "wasapi2src loopback=true name=audiocapsrc name=audiocapsrc !" 
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    "audioconvert ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    "audioresample ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    "opusenc name=audioencoder ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    "rtpopuspay ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    RTP_CAPS_OPUS "OPUS ! " "sendrecv. ", &error);

            pipe->audio_element[SOUND_SOURCE] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "audiocapsrc");
            pipe->video_element[H265_MEDIA_FOUNDATION] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "videoencoder");
            pipe->video_element[RTP_H265_PAYLOAD] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "rtp");
            pipe->video_element[DIRECTX_SCREEN_CAPTURE_SOURCE] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "screencap");
            pipe->audio_element[SOUND_ENCODER] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "audioencoder");
        }
    }
    else if (video == CODEC_VP9)
    {
        if (audio == OPUS_ENC)
        {
            pipe->pipeline =
                gst_parse_launch("webrtcbin bundle-policy=max-bundle name=sendrecv "
                    "d3d11desktopdupsrc name=screencap ! "DIRECTX_PAD",framerate=120/1 ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! "
                    "d3d11convert ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! "
                    "rtpvp9enc name=rtp ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    RTP_CAPS_VIDEO "VP9 ! sendrecv. "

                    "wasapi2src loopback=true name=audiocapsrc name=audiocapsrc !" 
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    "audioconvert ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    "audioresample ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    "opusenc name=audioencoder ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    "rtpopuspay ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    RTP_CAPS_OPUS "OPUS ! " "sendrecv. ", &error);


            pipe->audio_element[SOUND_SOURCE] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "audiocapsrc");
            pipe->video_element[VP9_ENCODER] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "videoencoder");
            pipe->video_element[RTP_VP9_PAYLOAD] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "rtp");
            pipe->video_element[DIRECTX_SCREEN_CAPTURE_SOURCE] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "screencap");
            pipe->audio_element[SOUND_ENCODER] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "audioencoder");
        }
    }
    else if (video == CODEC_VP8)
    {
        if (audio == OPUS_ENC)
        {
            pipe->pipeline =
                gst_parse_launch("webrtcbin bundle-policy=max-bundle name=sendrecv "
                    "dxgiscreencapsrc name=screencap ! "
                    " ! queue ! videoconvert ! queue ! "
                    "vp8enc name=videoencoder ! rtpvp8pay name=rtp ! "
                    "queue ! " RTP_CAPS_VIDEO "VP8 ! sendrecv. "

                    "wasapi2src loopback=true name=audiocapsrc name=audiocapsrc !" 
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    "audioconvert ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    "audioresample ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    "opusenc name=audioencoder ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    "rtpopuspay ! "
                    "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                    RTP_CAPS_OPUS "OPUS ! " "sendrecv. ", &error);


            pipe->audio_element[SOUND_SOURCE] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "audiocapsrc");
            pipe->video_element[VP8_ENCODER] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "videoencoder");
            pipe->video_element[RTP_VP8_PAYLOAD] =
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "rtp");
            pipe->video_element[DIRECTX_SCREEN_CAPTURE_SOURCE] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "screencap");
            pipe->audio_element[SOUND_ENCODER] = 
                gst_bin_get_by_name(GST_BIN(pipe->pipeline), "audioencoder");
        }
    }


    if(!pipe->pipeline)
    {
        pipe->pipeline =
            gst_parse_launch("webrtcbin bundle-policy=max-bundle name=sendrecv "
                "d3d11desktopdupsrc name=screencap ! "DIRECTX_PAD",framerate=60/1 ! "
                "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! "
                "d3d11convert ! "DIRECTX_PAD",format=NV12 ! "
                "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! "
                "mfh264enc name=videoencoder ! video/x-h264,profile=high ! "
                "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! "
                "rtph264pay name=rtp ! "
                "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                RTP_CAPS_VIDEO "H264 ! sendrecv. "

                "wasapi2src loopback=true name=audiocapsrc name=audiocapsrc !" 
                "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                "audioconvert ! "
                "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                "audioresample ! "
                "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                "opusenc name=audioencoder ! "
                "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                "rtpopuspay ! "
                "queue max-size-time=0 max-size-bytes=0 max-size-buffers=3 ! " 
                RTP_CAPS_OPUS "OPUS ! " "sendrecv. ", &error);

        pipe->audio_element[SOUND_SOURCE] = 
            gst_bin_get_by_name(GST_BIN(pipe->pipeline), "audiocapsrc");
        pipe->video_element[H264_MEDIA_FOUNDATION] = 
            gst_bin_get_by_name(GST_BIN(pipe->pipeline), "videoencoder");
        pipe->video_element[RTP_H264_PAYLOAD] = 
            gst_bin_get_by_name(GST_BIN(pipe->pipeline), "rtp");
        pipe->video_element[DIRECTX_SCREEN_CAPTURE_SOURCE] = 
            gst_bin_get_by_name(GST_BIN(pipe->pipeline), "screencap");
        pipe->audio_element[SOUND_ENCODER] = 
            gst_bin_get_by_name(GST_BIN(pipe->pipeline), "audioencoder");

    }
    if (error) {
        session_core_finalize(core,error);
    }
    pipe->webrtcbin =
        gst_bin_get_by_name(GST_BIN(pipe->pipeline), "sendrecv");
    
}








static void
device_foreach(GstDevice* device, 
                gpointer data)
{
    GstElement* element = (GstElement*) data;
    gchar* name = gst_device_get_display_name(device);
    gchar* class = gst_device_get_device_class(device);
    GstCaps* cap = gst_device_get_caps(device);
    GstStructure* cap_structure = gst_caps_get_structure (cap, 0);
    GstStructure* device_structure = gst_device_get_properties(device);
    gchar* api = gst_structure_get_string(device_structure,"device.api");
    gchar* id  = gst_structure_get_string(device_structure,"device.strid");
    if(!id)
        id  = gst_structure_get_string(device_structure,"device.id");


    gchar* cap_name = gst_structure_get_name (cap_structure);


    if(!g_strcmp0(api,"wasapi2"))
    {
        if(!g_strcmp0(class,"Audio/Source"))
        {
            if(!g_strcmp0(cap_name,"audio/x-raw"))
            {
                if(g_str_has_prefix(name,"CABLE Input"))
                {
                    GString* string = g_string_new("Selecting sound capture device: ");
                    g_string_append(string,name);
                    g_string_append(string," with device id: ");
                    g_string_append(string,id);
                    worker_log_output(g_string_free(string,FALSE));

                    memcpy(sound_output_device_id,id,strlen(id));
                }
            }
        }
    }

    if(!g_strcmp0(api,"wasapi2"))
    {
        if(!g_strcmp0(class,"Audio/Sink"))
        {
            if(!g_strcmp0(cap_name,"audio/x-raw"))
            {
                if(g_str_has_prefix(name,"CABLE"))
                {
                    GString* string = g_string_new("Selecting sound output device: ");
                    g_string_append(string,name);
                    g_string_append(string," with device id: ");
                    g_string_append(string,id);
                    worker_log_output(g_string_free(string,FALSE));

                    memcpy(sound_capture_device_id,id,strlen(id));
                }
            }
        }
    }

    gst_caps_unref(cap);
    g_object_unref(device);
}



/// <summary>
/// setup proerty of gst element,
/// this function should be called after pipeline factory has been done,
/// each element are assigned to an element in pipeline
/// </summary>
/// <param name="core"></param>
static void
setup_element_property(SessionCore* core)
{
    Pipeline* pipe = session_core_get_pipeline(core);
    SignallingHub* hub = session_core_get_signalling_hub(core);
    StreamConfig* qoe = session_core_get_qoe(core);


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (pipe->video_element[DIRECTX_SCREEN_CAPTURE_SOURCE]) { g_object_set(pipe->video_element[DIRECTX_SCREEN_CAPTURE_SOURCE], "show-cursor", TRUE, NULL);}
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (pipe->video_element[NVIDIA_H264_ENCODER]) { g_object_set(pipe->video_element[NVIDIA_H264_ENCODER], "bframes", 0, NULL);}

    if (pipe->video_element[NVIDIA_H264_ENCODER]) { g_object_set(pipe->video_element[NVIDIA_H264_ENCODER], "zerolatency", TRUE, NULL);}

    if (pipe->video_element[NVIDIA_H264_ENCODER]) { g_object_set(pipe->video_element[NVIDIA_H264_ENCODER], "rc-mode", "cbr", NULL);}

    if (pipe->video_element[NVIDIA_H264_ENCODER]) { g_object_set(pipe->video_element[NVIDIA_H264_ENCODER], "qos", TRUE, NULL);}

    if (pipe->video_element[NVIDIA_H264_ENCODER]) { g_object_set(pipe->video_element[NVIDIA_H264_ENCODER], "preset", "low-latency", NULL);}
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (pipe->video_element[H264_MEDIA_FOUNDATION]) { g_object_set(pipe->video_element[H264_MEDIA_FOUNDATION], "rc-mode", 0, NULL);}

    if (pipe->video_element[H264_MEDIA_FOUNDATION]) { g_object_set(pipe->video_element[H264_MEDIA_FOUNDATION], "quality-vs-speed", 100, NULL);}
    
    if (pipe->video_element[H264_MEDIA_FOUNDATION]) { g_object_set(pipe->video_element[H264_MEDIA_FOUNDATION], "low-latency", TRUE, NULL);}
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (pipe->video_element[H265_MEDIA_FOUNDATION]) { g_object_set(pipe->video_element[H265_MEDIA_FOUNDATION], "rc-mode", 0, NULL); }

    if (pipe->video_element[H265_MEDIA_FOUNDATION]) { g_object_set(pipe->video_element[H265_MEDIA_FOUNDATION], "quality-vs-speed", 100, NULL); }

    if (pipe->video_element[H265_MEDIA_FOUNDATION]) { g_object_set(pipe->video_element[H265_MEDIA_FOUNDATION], "bitrate", 5000, NULL); }

    if (pipe->video_element[H265_MEDIA_FOUNDATION]) { g_object_set(pipe->video_element[H265_MEDIA_FOUNDATION], "low-latency", TRUE, NULL); }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (pipe->video_element[RTP_H264_PAYLOAD]) { g_object_set(pipe->video_element[RTP_H264_PAYLOAD], "aggregate-mode", 1, NULL);}
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (pipe->video_element[RTP_H265_PAYLOAD]) { g_object_set(pipe->video_element[RTP_H265_PAYLOAD], "aggregate-mode", 1, NULL); }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (pipe->audio_element[SOUND_SOURCE]) { g_object_set(pipe->audio_element[SOUND_SOURCE], "low-latency", TRUE, NULL);}

    if (pipe->audio_element[SOUND_SOURCE]) { g_object_set(pipe->audio_element[SOUND_SOURCE], "device", sound_capture_device_id, NULL);}
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (pipe->audio_element[RTP_RTX_QUEUE]) { g_object_set(pipe->audio_element[RTP_RTX_QUEUE], "max-size-time", 16000000, NULL);}

    if (pipe->audio_element[RTP_RTX_QUEUE]) { g_object_set(pipe->audio_element[RTP_RTX_QUEUE], "max-size-packet", 0, NULL);}
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    g_object_set(pipe->webrtcbin,"latency",0,NULL);
}


void
toggle_pointer(gboolean toggle, SessionCore* core)
{
    Pipeline* pipe = session_core_get_pipeline(core);
    if (pipe->video_element[DIRECTX_SCREEN_CAPTURE_SOURCE]) 
    { 
        g_object_set(pipe->video_element[DIRECTX_SCREEN_CAPTURE_SOURCE], "show-cursor", toggle, NULL); 
    }
}




void
setup_pipeline(SessionCore* core)
{
    SignallingHub* signalling = session_core_get_signalling_hub(core);
    Pipeline* pipe = session_core_get_pipeline(core);
    StreamConfig* qoe= session_core_get_qoe(core);
    


    setup_element_factory(core, 
        qoe_get_video_codec(qoe),
        qoe_get_audio_codec(qoe));
    

    signalling_hub_setup_turn_and_stun(pipe,signalling);
    connect_signalling_handler(core);
    setup_element_property(core);


    gst_element_change_state(pipe->pipeline, GST_STATE_READY);
    connect_data_channel_signals(core);
    start_pipeline(core);
}






GstElement*
pipeline_get_webrtc_bin(Pipeline* pipe)
{
    return pipe->webrtcbin;
}