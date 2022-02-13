
#include <session-webrtc-type.h>
#include <session-webrtc.h>
#include <session-webrtc-pipeline.h>
#include <session-webrtc-data-channel.h>

#include <gst/gst.h>
#include <gst/webrtc/webrtc.h>
#include <logging.h>

#ifdef G_OS_WIN32
#include <Windows.h>
#endif








GstWebRTCPeerConnectionState
webrtcbin_get_qos(GstElement* webrtcbin)
{
    GstWebRTCPeerConnectionState state;
    g_object_get(webrtcbin,"connection-state",&state,NULL);

    GstPromise* promise = gst_promise_new();
    g_signal_emit_by_name(webrtcbin,"get-stats",NULL,promise);
    GstPromiseResult result = gst_promise_wait(promise);
    if(result = GST_PROMISE_RESULT_REPLIED)
    {
        GstStructure* structure = gst_promise_get_reply(promise);
        gchar* name = gst_structure_get_name(structure);

#ifdef G_OS_WIN32
        gchar* output = gst_structure_serialize(structure,GST_SERIALIZE_FLAG_NONE);
#else
        gchar* output = gst_structure_to_string(structure);
#endif
        worker_log_output(output);
    }
    gst_promise_unref(promise);
    return state;
}



static gpointer
handle_webrtc_connection_thread(gpointer data)
{
    SessionCore* core = (SessionCore*) data;
    Pipeline* pipeline = session_core_get_pipeline(core);

    while (TRUE)
    {
        GstWebRTCPeerConnectionState state = webrtcbin_get_qos(pipeline_get_webrtc_bin(pipeline));
        if( state == GST_WEBRTC_PEER_CONNECTION_STATE_DISCONNECTED ||
            state == GST_WEBRTC_PEER_CONNECTION_STATE_FAILED ||
            state == GST_WEBRTC_PEER_CONNECTION_STATE_CLOSED )
        {
            worker_log_output("client closed, session stop");
            session_core_finalize(core,NULL);
        }
#ifdef G_OS_WIN32
        Sleep(500);
#endif
    }
}

void
start_qos_thread(SessionCore* core)
{
    g_thread_new("QoS",
        (GThreadFunc)handle_webrtc_connection_thread, core);
}
