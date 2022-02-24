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
#include <handle-key.h>
#include <enum.h>

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
};

typedef struct _Device
{
    gchar sound_capture_device_id[1000];

    gchar sound_output_device_id[1000];

    guint64 monitor_handle;
}Device;

static Device capture_device = {0};


void device_foreach(GstDevice* data, gpointer user_data);

/**
 * @brief 
 * get and set the visibility of pointer
 * @param capture 
 */
static void
toggle_pointer(GstElement* capture)
{
    gboolean toggle;
    g_object_get(capture, "show-cursor", &toggle, NULL); 
    toggle = !toggle;
    set_relative_mouse(!toggle);
    g_object_set(capture, "show-cursor", toggle, NULL); 
}

Pipeline*
pipeline_initialize()
{
    Pipeline* pipeline = malloc(sizeof(Pipeline));
    memset(pipeline,0,sizeof(Pipeline));

#ifdef G_OS_WIN32
    GstDeviceMonitor* monitor = gst_device_monitor_new();
    if(!gst_device_monitor_start(monitor)) {
        worker_log_output("WARNING: Monitor couldn't started!!\n");
    }

    worker_log_output("Searching for available device");
    GList* device_list = gst_device_monitor_get_devices(monitor);
    g_list_foreach(device_list,(GFunc)device_foreach,NULL);
#endif
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

static Shortcut*
get_default_shortcut(SessionCore* core)
{
    Pipeline* pipe = session_core_get_pipeline(core);
    Shortcut* shortcuts = malloc(sizeof(Shortcut)*10);
    memset(shortcuts,0,sizeof(Shortcut)*10);

    (shortcuts + 0)->data = pipe->video_element[SCREEN_CAPTURE];
    (shortcuts + 0)->function = toggle_pointer;
    (shortcuts + 0)->opcode = POINTER_LOCK;
    (shortcuts + 0)->active = TRUE;

    return shortcuts;
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

    Shortcut* shortcuts = get_default_shortcut(core);
    pipe->handler = activate_hid_handler(pipe->video_element[SCREEN_CAPTURE],shortcuts);
	free(shortcuts);


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








void
device_foreach(GstDevice* device, 
                gpointer data)
{
    GstElement* element = (GstElement*) data;
    gchar* name = gst_device_get_display_name(device);
    gchar* class = gst_device_get_device_class(device);
    GstCaps* cap = gst_device_get_caps(device);
    GstStructure* cap_structure = gst_caps_get_structure (cap, 0);
    GstStructure* device_structure = gst_device_get_properties(device);
    gchar* cap_name = gst_structure_get_name (cap_structure);
    gchar* api = gst_structure_get_string(device_structure,"device.api");
    
    if(!g_strcmp0(api,"wasapi2"))
    {
        gchar* id  = gst_structure_get_string(device_structure,"device.strid");
        id = id ? id: gst_structure_get_string(device_structure,"device.id");
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

                    memcpy(capture_device.sound_output_device_id,id,strlen(id));
                }
            }
        }

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
                    memcpy(capture_device.sound_capture_device_id,id,strlen(id));
                }
            }
        }
    }


    if(!g_strcmp0(api,"d3d11"))
    {
        if(!g_strcmp0(class,"Source/Monitor"))
        {
            gint width, height; 
            guint64 id; 
            gst_structure_get_int(cap_structure,"width",&width);
            gst_structure_get_int(cap_structure,"height",&height);
            gst_structure_get_uint64(device_structure,"device.hmonitor",&id);

            if(!g_strcmp0(name,"Linux FHD"))
                capture_device.monitor_handle = id;
        }
    }

    gst_caps_unref(cap);
    g_object_unref(device);
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

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef G_OS_WIN32
    if (pipe->video_element[SCREEN_CAPTURE]) { g_object_set(pipe->video_element[SCREEN_CAPTURE], "show-cursor", FALSE, NULL);}
#else
    if (pipe->video_element[SCREEN_CAPTURE]) { g_object_set(pipe->video_element[SCREEN_CAPTURE], "show-pointer", TRUE, NULL);}

    if (pipe->video_element[SCREEN_CAPTURE]) { g_object_set(pipe->video_element[SCREEN_CAPTURE], "remote", TRUE, NULL);}

    if (pipe->video_element[SCREEN_CAPTURE]) { g_object_set(pipe->video_element[SCREEN_CAPTURE], "blocksize", 16384, NULL);}
    
    if (pipe->video_element[SCREEN_CAPTURE]) { g_object_set(pipe->video_element[SCREEN_CAPTURE], "use-damage", FALSE, NULL);}
#endif
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef G_OS_WIN32
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (pipe->video_element[RTP_VIDEO_PAYLOAD]) { g_object_set(pipe->video_element[RTP_VIDEO_PAYLOAD], "aggregate-mode", 1, NULL);}
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef G_OS_WIN32
    if (pipe->audio_element[SOUND_SOURCE]) { g_object_set(pipe->audio_element[SOUND_SOURCE], "provide-clock", TRUE, NULL);}

    if (pipe->audio_element[SOUND_SOURCE]) { g_object_set(pipe->audio_element[SOUND_SOURCE], "do-timestamp", TRUE, NULL);}
#else
    if (pipe->audio_element[SOUND_SOURCE]) { g_object_set(pipe->audio_element[SOUND_SOURCE], "low-latency", TRUE, NULL);}
#endif

    if(strlen(capture_device.sound_capture_device_id))
    {
        if (pipe->audio_element[SOUND_SOURCE]) 
        {
            g_object_set(pipe->audio_element[SOUND_SOURCE], "device", capture_device.sound_capture_device_id, NULL);
        }
    }

    if (capture_device.monitor_handle) 
    {
        if (pipe->video_element[SCREEN_CAPTURE]) 
        {
            g_object_set(pipe->video_element[SCREEN_CAPTURE], "monitor-handle", capture_device.monitor_handle, NULL);
        }
    }

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