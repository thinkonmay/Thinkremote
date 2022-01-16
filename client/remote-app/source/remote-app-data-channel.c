#include <remote-app-data-channel.h>
#include <remote-app.h>
#include <remote-app-type.h>
#include <remote-app-pipeline.h>
#include <remote-app-remote-config.h>
#include <remote-app-signalling.h>


#include <human-interface-opcode.h>
#include <module-code.h>
#include <development.h>

#include <gst/gst.h>
#include <glib-2.0/glib.h>
#include <gst/webrtc/webrtc_fwd.h>
#include <json-glib/json-glib.h>
#include <gst/webrtc/webrtc.h>
#include <gst/sdp/sdp.h>












struct _WebRTCHub
{
    /**
     * @brief 
     * hid data channel for human interface device
     */
    GObject* hid;

    /**
     * @brief 
     * control datachannel for control signal
     */
    GObject* control;
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
 * 
 */
static gboolean ping = TRUE;

/**
 * @brief 
 * 
 * @param data 
 * @return gpointer 
 */
gpointer
ping_thread(gpointer data)
{
    RemoteApp* app = (RemoteApp*) data;
    while (TRUE)
    {
#ifdef G_OS_WIN32
        Sleep(2000);
#else
        sleep(2000);
#endif

        if(ping)
        {
            ping = FALSE;
            continue;
        }
        else
        {
            SignallingHub* hub = remote_app_get_signalling_hub(app);
            signalling_close(hub);
            setup_pipeline(app);
            signalling_connect(app);
        }
    }
}


void
hid_data_channel_send(gchar* message,
                      RemoteApp* app)
{
    WebRTCHub* hub = remote_app_get_rtc_hub(app);
    if(DEVELOPMENT_ENVIRONMENT)
    {
        g_print(message);
        g_print("\n");
    }

    if(!g_strcmp0(message,"ping"))
    {
        g_signal_emit_by_name(hub->hid,"send-string",message);
    }
}


void
control_data_channel_send(gchar* message,
                          RemoteApp* core)
{
    WebRTCHub* hub = remote_app_get_rtc_hub(core);
    g_signal_emit_by_name(hub->control,"send-string",message);
}










/**
 * @brief 
 * 
 * @param datachannel 
 * @param data 
 * @param core 
 */
static void
data_channel_on_message_data(GObject* datachannel,
    GBytes* data,
    RemoteApp* core)
{
    return;
}


/**
 * @brief 
 * handle message string from hid data channel
 * @param dc 
 * @param message 
 * @param core 
 */
static void
hid_channel_on_message_string(GObject* dc,
    gchar* message,
    RemoteApp* core)
{
    QoE* qoe = remote_app_get_qoe(core);

    GError* error = NULL;
    JsonParser* parser = json_parser_new();
    JsonObject* object = get_json_object_from_string(message,&error,parser);
	if(!error == NULL || object == NULL) {return;}

    gint opcode = json_object_get_int_member(object, "Opcode");
    g_object_unref(parser);
}


/**
 * @brief 
 * handle message from control data channel
 * @param dc 
 * @param message 
 * @param core 
 */
static void
control_channel_on_message_string(GObject* dc,
    gchar* message,
    RemoteApp* core)
{
    if(!g_strcmp0(message,"ping"))
        g_signal_emit_by_name(dc,"send-string","ping",NULL);
}




static void
channel_on_open(GObject* dc,
                RemoteApp* core)
{
    return;
}


static void
channel_on_close_and_error(GObject* dc,
    RemoteApp* core)
{
    return;
}










/**
 * @brief 
 * handle new data channel created by worker node
 * @param webrtc webrtcbin
 * @param channel new datachannel
 * @param data remote app
 * @return gboolean 
 */
static gboolean
on_data_channel(GstElement* webrtc,
    GObject* channel,
    gpointer data)
{
    RemoteApp* core = (RemoteApp*)data;
    WebRTCHub* hub = remote_app_get_rtc_hub(core);
    gchar* label;
    g_object_get(channel,"label",&label,NULL);

    if(!g_strcmp0(label,"Control"))
    {
        hub->control = channel;
        g_signal_connect(hub->control, "on-error",
            G_CALLBACK(channel_on_close_and_error), core);
        g_signal_connect(hub->control, "on-open",
            G_CALLBACK(channel_on_open), core);
        g_signal_connect(hub->control, "on-close",
            G_CALLBACK(channel_on_close_and_error), core);
        g_signal_connect(hub->control, "on-message-string",
            G_CALLBACK(control_channel_on_message_string), core);
        g_signal_connect(hub->control, "on-message-data",
            G_CALLBACK(data_channel_on_message_data), core);
    }
    else if (!g_strcmp0(label,"HID"))
    {
        hub->hid = channel;
        g_signal_connect(hub->hid, "on-error",
            G_CALLBACK(channel_on_close_and_error), core);
        g_signal_connect(hub->hid, "on-open",
            G_CALLBACK(channel_on_open), core);
        g_signal_connect(hub->hid, "on-close",
            G_CALLBACK(channel_on_close_and_error), core);
        g_signal_connect(hub->hid, "on-message-string",
            G_CALLBACK(hid_channel_on_message_string), core);
        g_signal_connect(hub->hid, "on-message-data",
            G_CALLBACK(data_channel_on_message_data), core);
    }
    else
    {
        return;
    }
}



gboolean
connect_data_channel_signals(RemoteApp* core)
{

    WebRTCHub* hub = remote_app_get_rtc_hub(core);
    Pipeline* pipe = remote_app_get_pipeline(core);
    GstElement* webrtcbin = pipeline_get_webrtc_bin(pipe);

    // connect data channel source
    g_signal_connect(webrtcbin, "on-data-channel",
        G_CALLBACK(on_data_channel), core);
}


