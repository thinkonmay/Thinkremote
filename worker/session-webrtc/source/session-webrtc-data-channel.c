/**
 * @file session-webrtc-data-channel.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <session-webrtc-data-channel.h>
#include <session-webrtc.h>
#include <session-webrtc-type.h>
#include <session-webrtc-pipeline.h>


#include <logging.h>
#include <enum.h>
#include <key-convert.h>
#include <constant.h>
#include <remote-config.h>

#include <global-var.h>

#include <gst/gst.h>
#include <glib-2.0/glib.h>
#include <gst/webrtc/webrtc_fwd.h>
#include <libsoup/soup.h>

#ifdef G_OS_WIN32
#include <handle-key.h>
#endif

struct _WebRTCHub
{
    /**
     * @brief 
     * hid datachannel for transfering human interface signal with client 
     */
    GObject* hid;

    /**
     * @brief 
     * control datachannel for transfering control signal with client
     */
    GObject* control;


    /**
     * @brief 
     * 
     */
    DeviceType device; 
    
    /**
     * @brief 
     * 
     */
    CoreEngine engine; 
};


WebRTCHub* 
webrtchub_initialize()
{
    WebRTCHub* hub = malloc(sizeof(WebRTCHub));
    memset(hub,0,sizeof(WebRTCHub));
    return hub;
}


/**
 * @brief 
 * ignore binary message
 * @param datachannel 
 * @param byte 
 * @param core 
 */
static void
control_channel_on_message_data(GObject* datachannel,
    GBytes* byte,
    SessionCore* core)
{
    return;
}

/**
 * @brief 
 * ignore binary message
 * @param datachannel 
 * @param data 
 * @param core 
 */
static void
hid_channel_on_message_data(GObject* datachannel,
    GBytes* data,
    SessionCore* core)
{
    return;
}


void
send_hid_message(SessionCore* core,
                 gchar* message)
{
    WebRTCHub* hub = session_core_get_rtc_hub(core);
    g_signal_emit_by_name(hub->hid,"send-string",message,NULL);
}



/**
 * @brief 
 * handle message from hid datachannel and send to window
 * @param dc 
 * @param message 
 * @param core 
 */
static void
hid_channel_on_message_string(GObject* dc,
                            gchar* message,
                            SessionCore* core)
{
    WebRTCHub* hub = session_core_get_rtc_hub(core);
#ifdef G_OS_WIN32
    if(hub->device == WEB_APP && hub->engine == CHROME)
        handle_input_javascript(message);
    else if(hub->engine == GSTREAMER && hub->device == WINDOW_APP)
        handle_input_win32(message,(MousePositionFeedbackFunc)send_hid_message,core);
#endif
}






/**
 * @brief 
 * handle control data channel
 * @param dc 
 * @param message 
 * @param core 
 */
static void
control_channel_on_message_string(GObject* dc,
                                  gchar* message,
                                  SessionCore* core)
{
    if(!g_strcmp0(message,"ping"))
    {
        g_signal_emit_by_name(dc,"send-string",message,NULL);
        return;
    }

    WebRTCHub* hub = session_core_get_rtc_hub(core);
    JsonParser* parser = json_parser_new();
    JsonObject* object = get_json_object_from_string(message,NULL,parser);

    if(!object)
        goto free;

    hub->device =  json_object_get_int_member(object,"Device");
    hub->engine =  json_object_get_int_member(object,"Engine");
free:
    g_object_unref(parser);
}


/**
 * @brief 
 * handle datachannel open signal
 * @param dc 
 * @param core 
 */
static void
channel_on_open(GObject* dc,
                SessionCore* core)
{
    return;
}


/**
 * @brief 
 * handle datachannel close and error signal
 * @param dc 
 * @param core 
 */
static void
channel_on_close_and_error(GObject* dc,
    SessionCore* core)
{
    return;
}







gboolean
connect_data_channel_signals(SessionCore* core)
{
    WebRTCHub* hub = session_core_get_rtc_hub(core);
    Pipeline* pipe = session_core_get_pipeline(core);
    GstElement* webrtcbin = pipeline_get_webrtc_bin(pipe);


    // connect data channel source
    g_signal_emit_by_name(webrtcbin, "create-data-channel", "Control", 
        NULL, &hub->control);
    g_signal_emit_by_name(webrtcbin, "create-data-channel", "HID", 
        NULL, &hub->hid);
    
    g_signal_connect(hub->hid, "on-error",
        G_CALLBACK(channel_on_close_and_error), core);
    g_signal_connect(hub->hid, "on-open",
        G_CALLBACK(channel_on_open), core);
    g_signal_connect(hub->hid, "on-close",
        G_CALLBACK(channel_on_close_and_error), core);
    g_signal_connect(hub->hid, "on-message-string",
        G_CALLBACK(hid_channel_on_message_string), core);
    g_signal_connect(hub->hid, "on-message-data",
        G_CALLBACK(hid_channel_on_message_data), core);

    g_signal_connect(hub->control, "on-error",
        G_CALLBACK(channel_on_close_and_error), core);
    g_signal_connect(hub->control, "on-open",
        G_CALLBACK(channel_on_open), core);
    g_signal_connect(hub->control, "on-close",
        G_CALLBACK(channel_on_close_and_error), core);
    g_signal_connect(hub->control, "on-message-string",
        G_CALLBACK(control_channel_on_message_string), core);
    g_signal_connect(hub->control, "on-message-data",
        G_CALLBACK(control_channel_on_message_data), core);
    return TRUE;
}



GObject*
webrtc_hub_get_control_data_channel(WebRTCHub* hub)
{
    return hub->control;
}