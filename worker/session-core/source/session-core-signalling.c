/**
 * @file session-core-signalling.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <session-core-signalling.h>
#include <session-core.h>
#include <session-core-pipeline.h>
#include <session-core-type.h>

#include <logging.h>
#include <signalling-message.h>
#include <global-var.h>
#include <development.h>

#include <gst/gst.h>
#include <glib-2.0/glib.h>
#include <session-core-type.h>
#include <libsoup/soup.h>
#include <json-glib/json-glib.h>

#include <gst/webrtc/webrtc.h>
#include <gst/rtp/gstrtppayloads.h>
#include <libsoup/soup.h>

#include <stdio.h>




struct _SignallingHub
{
    /**
     * @brief 
     * websocket connection instance to connect with signalling server
     */
    SoupWebsocketConnection* connection;

    /**
     * @brief 
     * 
     * soup session represent a session between session core and signalling server,
     * it encapsulate signalling url and disable ssl option
     */
    SoupSession* session;

    /**
     * @brief 
     * 
     */
    gchar signalling_server[100];

    /**
     * @brief 
     * turn connection of the session 
     */
	gchar turn[200];

    /**
     * @brief 
     * remote token use to establish connection with client
     */
    gchar remote_token[500];

    /**
     * @brief 
     * list of stun server
     */
    gchar stuns[10][50];
};


void
on_server_connected(SoupSession* session,
    GAsyncResult* res,
    SessionCore* core);


SignallingHub*
signalling_hub_initialize(SessionCore* core)
{
    SignallingHub* hub = malloc(sizeof(SignallingHub));
    memset(hub, 0, sizeof(SignallingHub));
    return hub;
}

static void
handle_stun_list(JsonArray* stun_array,
                 gint index,
                 JsonNode* node,
                 gpointer data)
{
    SignallingHub* hub = (SignallingHub*)data;
    gchar* value = json_array_get_string_element(stun_array,index);
    GString* stun = g_string_new("stun://");
    g_string_append(stun,value);
    gchar* stun_url = g_string_free(stun,FALSE);
    memcpy(hub->stuns[index], stun_url, strlen(stun_url));
}

void
signalling_hub_setup(SignallingHub* hub, 
                     gchar* turn,
                     gchar* url,
                     JsonArray* stun_array,
                     gchar* remote_token)
{
    if(!g_strcmp0(turn,"turn://:@turn::3478"))
    {
		worker_log_output("Fail to get turn server, setting default value");
        turn = DEFAULT_TURN;
    }
    else
    {
		worker_log_output("starting remote session with turn server");
		worker_log_output(turn);
    }

    memcpy(hub->remote_token, remote_token,strlen(remote_token));
    memcpy(hub->signalling_server, url,strlen(url));
    memcpy(hub->turn, turn,strlen(turn));

    if(stun_array)
    {
        json_array_foreach_element(stun_array,
            (JsonArrayForeach)handle_stun_list,(gpointer)hub);
    }
    else
    {
        worker_log_output("no stun server found, setting default value\n");
    }
}


void
send_message_to_signalling_server(SignallingHub* signalling,
                                gchar* request_type,
                                gchar* content)
{
    JsonObject* json_object = json_object_new();
    json_object_set_string_member(json_object, REQUEST_TYPE, request_type);
    json_object_set_string_member(json_object, CONTENT, content);

    
    gchar* buffer = get_string_from_json_object(json_object);
    json_object_unref(json_object);
    worker_log_output( buffer);
    soup_websocket_connection_send_text(signalling->connection,buffer);
    g_free(buffer);
}



void
send_ice_candidate_message(GstElement* webrtc G_GNUC_UNUSED,
    guint mlineindex,
    gchar* candidate,
    SessionCore* core G_GNUC_UNUSED)
{
    gchar* text;
    JsonObject* ice, * msg;

    SignallingHub* hub = session_core_get_signalling_hub(core);


    ice = json_object_new();
    json_object_set_string_member(ice, "candidate", candidate);
    json_object_set_int_member(ice, "sdpMLineIndex", mlineindex);
    msg = json_object_new();
    json_object_set_object_member(msg, "ice", ice);
    text = get_string_from_json_object(msg);



    send_message_to_signalling_server(hub,OFFER_ICE,text);
    g_free(text);
}



void
send_sdp_to_peer(SessionCore* core,
    GstWebRTCSessionDescription* desc)
{
    gchar* text;
    JsonObject* msg, * sdp;

    SignallingHub* hub = session_core_get_signalling_hub(core);

    text = gst_sdp_message_as_text(desc->sdp);
    sdp = json_object_new();

    if (desc->type == GST_WEBRTC_SDP_TYPE_OFFER) {
        json_object_set_string_member(sdp, "type", "offer");
    } else if (desc->type == GST_WEBRTC_SDP_TYPE_ANSWER) {
        json_object_set_string_member(sdp, "type", "answer");
    } else {
        g_assert_not_reached();
    }

    json_object_set_string_member(sdp, "sdp", text);
    g_free(text);

    msg = json_object_new();
    json_object_set_object_member(msg, "sdp", sdp);
    text = get_string_from_json_object(msg);
    json_object_unref(msg);

    send_message_to_signalling_server(hub,OFFER_SDP,text);
    g_free(text);
}


/* Offer created by our pipeline, to be sent to the peer */
void
on_offer_created( GstPromise* promise, SessionCore* core)
{
    GstWebRTCSessionDescription* offer = NULL;
    const GstStructure* reply;

    Pipeline* pipe = session_core_get_pipeline(core);
    SignallingHub* hub = session_core_get_signalling_hub(core);


    g_assert_cmphex(gst_promise_wait(promise), == , GST_PROMISE_RESULT_REPLIED);

    reply = gst_promise_get_reply(promise);
    gst_structure_get(reply, "offer",
        GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &offer, NULL);
    gst_promise_unref(promise);

    promise = gst_promise_new();
    g_signal_emit_by_name(pipeline_get_webrtc_bin(pipe),
        "set-local-description", offer, promise);

    gst_promise_interrupt(promise);
    gst_promise_unref(promise);

    /* Send offer to peer */
    send_sdp_to_peer(core,offer);
    gst_webrtc_session_description_free(offer);
}


void
on_negotiation_needed(GstElement* element, SessionCore* core)
{
    Pipeline* pipe = session_core_get_pipeline(core);
    SignallingHub* signalling = session_core_get_signalling_hub(core);


    GstPromise* promise =
    gst_promise_new_with_change_func(on_offer_created, core, NULL);

    g_signal_emit_by_name(pipeline_get_webrtc_bin(pipe),
        "create-offer", NULL, promise);

}



void
on_ice_gathering_state_notify(GstElement* webrtcbin,
    GParamSpec* pspec,
    gpointer user_data)
{
    GstWebRTCICEGatheringState ice_gather_state;
    const gchar* new_state = "unknown";

    g_object_get(webrtcbin, "ice-gathering-state", &ice_gather_state, NULL);
    switch (ice_gather_state) {
    case GST_WEBRTC_ICE_GATHERING_STATE_NEW:
        new_state = "new";
        break;
    case GST_WEBRTC_ICE_GATHERING_STATE_GATHERING:
        new_state = "gathering";
        break;
    case GST_WEBRTC_ICE_GATHERING_STATE_COMPLETE:
        new_state = "complete";
        break;
    }
}




/// <summary>
/// close
/// </summary>
/// <param name="G_GNUC_UNUSED"></param>
/// <param name="G_GNUC_UNUSED"></param>
void
on_server_closed(SoupWebsocketConnection* conn G_GNUC_UNUSED,
    SessionCore* core G_GNUC_UNUSED)
{
    SignallingHub* hub = session_core_get_signalling_hub(core);
    hub->connection = NULL;
    hub->session = NULL;
    session_core_finalize(core,NULL);
}

/* Answer created by our pipeline, to be sent to the peer */
void
on_answer_created(GstPromise* promise,
    SessionCore* core)
{
    GstWebRTCSessionDescription* answer = NULL;
    const GstStructure* reply;

    Pipeline* pipe = session_core_get_pipeline(core);
    SignallingHub* hub = session_core_get_signalling_hub(core);


    g_assert_cmphex(gst_promise_wait(promise), == , GST_PROMISE_RESULT_REPLIED);
    reply = gst_promise_get_reply(promise);
    gst_structure_get(reply, "answer",
        GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &answer, NULL);
    gst_promise_unref(promise);

    promise = gst_promise_new();

    // if(!answer || !answer->sdp || !answer->type) {return;}

    g_signal_emit_by_name( pipeline_get_webrtc_bin(pipe),
        "set-local-description", answer, promise);

    gst_promise_interrupt(promise);
    gst_promise_unref(promise);

    /* Send answer to peer */
    send_sdp_to_peer(core,answer);
    gst_webrtc_session_description_free(answer);
}

void
on_offer_set(GstPromise* promise,
    SessionCore* core)
{
    Pipeline* pipe = session_core_get_pipeline(core);
    GstElement* webrtc = pipeline_get_webrtc_bin(pipe);

    gst_promise_unref(promise);
    promise = gst_promise_new_with_change_func(on_answer_created, 
        core, NULL);

    g_signal_emit_by_name(webrtc,
        "create-answer", NULL, promise);
}

void
on_offer_received(SessionCore* core, GstSDPMessage* sdp)
{
    GstWebRTCSessionDescription* offer = NULL;
    GstPromise* promise;

    Pipeline* pipe = session_core_get_pipeline(core);

    offer = gst_webrtc_session_description_new(GST_WEBRTC_SDP_TYPE_OFFER, sdp);
    g_assert_nonnull(offer);

    /* Set remote description on our pipeline */
    promise = gst_promise_new_with_change_func(on_offer_set, 
        core, NULL);

    g_signal_emit_by_name(pipeline_get_webrtc_bin(pipe), 
        "set-remote-description", offer,
        promise);
    gst_webrtc_session_description_free(offer);
}




void
session_core_logger(SoupLogger* logger,
            SoupLoggerLogLevel  level,
            char                direction,
            const char         *data,
            gpointer            user_data)
{
    worker_log_output(data);
}








void
signalling_connect(SessionCore* core)
{
    SoupLogger* logger;
    SoupMessage* message;

    const char* https_aliases[] = { "wss", NULL };
    JsonObject* json_object;

    SignallingHub* hub = session_core_get_signalling_hub(core);
    gchar* text;



    hub->session =
        soup_session_new_with_options(SOUP_SESSION_SSL_STRICT, TRUE,
            SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
            SOUP_SESSION_HTTPS_ALIASES, https_aliases, NULL);

    logger = soup_logger_new(SOUP_LOGGER_LOG_BODY, -1);

    GString* request_uri = g_string_new(hub->signalling_server);
    g_string_append(request_uri,"?token=");
    g_string_append(request_uri,hub->remote_token);
    gchar* request_string = g_string_free(request_uri,FALSE);

    message = soup_message_new(SOUP_METHOD_GET, request_string);

    worker_log_output("connecting to signalling server");

    soup_session_websocket_connect_async(hub->session,
        message, NULL, NULL, NULL,
        (GAsyncReadyCallback)on_server_connected, core);
}


static void
on_registering_message(SessionCore* core)
{
    SignallingHub* signalling = session_core_get_signalling_hub(core);
    /* Call has been setup by the server, now we can start negotiation */
}

static void
on_ice_exchange(gchar* text,SessionCore* core)
{
    Pipeline* pipe = session_core_get_pipeline(core);

    GError* error = NULL;
    JsonParser* parser = json_parser_new();
    JsonObject* object = get_json_object_from_string(text,&error,parser);
	if(!error == NULL || object == NULL) {return;}

    const gchar* candidate;
    gint sdpmlineindex;
    JsonObject* child = json_object_get_object_member(object, "ice");
    candidate = json_object_get_string_member(child, "candidate");
    sdpmlineindex = json_object_get_int_member(child, "sdpMLineIndex");
    /* Add ice candidate sent by remote peer */
    g_signal_emit_by_name(pipeline_get_webrtc_bin(pipe),
        "add-ice-candidate", sdpmlineindex, candidate);
    g_object_unref(parser);
}

static void
on_sdp_exchange(gchar* data, 
                SessionCore* core)
{
    SignallingHub* hub = session_core_get_signalling_hub(core);
    Pipeline* pipe = session_core_get_pipeline(core);

    GError* error = NULL;
    JsonParser* parser = json_parser_new();
    JsonObject* object = get_json_object_from_string(data,&error,parser);
	if(!error == NULL || object == NULL) {session_core_finalize(core,error);}

    gint ret;
    GstSDPMessage* sdp;
    gchar* text;
    GstWebRTCSessionDescription* answer;

    JsonObject* child = json_object_get_object_member(object, "sdp");
    gchar* sdptype = json_object_get_string_member(child, "type");

    if (!json_object_has_member(child, "type"))
    {
        worker_log_output("signalling error");
        return;
    }


    text = json_object_get_string_member(child, "sdp");
    ret = gst_sdp_message_new(&sdp);

    g_assert_cmphex(ret, == , GST_SDP_OK);
    ret = gst_sdp_message_parse_buffer((guint8*)text, strlen(text), sdp);
    g_assert_cmphex(ret, == , GST_SDP_OK);


    if (g_str_equal(sdptype, "answer"))
    {
        answer = gst_webrtc_session_description_new(GST_WEBRTC_SDP_TYPE_ANSWER,sdp);

        g_assert_nonnull(answer);
        /* Set remote description on our pipeline */
        {
            GstPromise* promise = gst_promise_new();
            g_signal_emit_by_name(pipeline_get_webrtc_bin(pipe),
                "set-remote-description", answer, promise);
            gst_promise_interrupt(promise);
            gst_promise_unref(promise);
        }
    }
    else
    {
        on_offer_received(core,sdp);
    }
    g_object_unref(parser);
}

/// <summary>
/// callback function for signalling server message
/// 
/// </summary>
/// <param name="conn"></param>
/// <param name="type"></param>
/// <param name="message"></param>
/// <param name="core"></param>
void
on_server_message(SoupWebsocketConnection* conn,
    SoupWebsocketDataType type,
    GBytes* message,
    SessionCore* core)
{
    Pipeline* pipe = session_core_get_pipeline(core);
    gchar* text;

    switch (type) 
    {
        case SOUP_WEBSOCKET_DATA_BINARY:
        {
            worker_log_output("Unknown message");
            return;
        } 
        case SOUP_WEBSOCKET_DATA_TEXT: 
        {
            gsize size;
            const char* data = g_bytes_get_data(message, &size);
            /* Convert to NULL-terminated string */
            text = g_strndup(data, size);
            worker_log_output(text);
            break;
        }
        default:
            worker_log_output("Unknown message");
    }


    GError* error = NULL;
    JsonParser* parser = json_parser_new();
    JsonObject* object = get_json_object_from_string(text,&error,parser);
	if(!error == NULL || object == NULL) {return;}

    gchar* RequestType =    json_object_get_string_member(object, "RequestType");
    gchar* Content =        json_object_get_string_member(object, "Content");

    /*this is websocket message with signalling server and has nothing to do with 
    * json message format use to communicate with other module
    */
    if (!g_strcmp0(RequestType, "OFFER_SDP")) {
        on_sdp_exchange(Content, core);
    } else if (!g_strcmp0(RequestType, "OFFER_ICE")) {
        on_ice_exchange(Content, core);
    } else if (!g_strcmp0(RequestType, "REQUEST_STREAM")) {
        setup_pipeline(core);
    }
    g_free(text);
}


void
on_server_connected(SoupSession* session,
    GAsyncResult* res,
    SessionCore* core)
{
    GError* error = NULL;
    SignallingHub* hub = session_core_get_signalling_hub(core);

    
    hub->connection = soup_session_websocket_connect_finish(session, res, &error);
    if (!error == NULL || hub->connection == NULL) { session_core_finalize(core, error); }

    worker_log_output("connected with signalling server");
    g_signal_connect(hub->connection, "closed", G_CALLBACK(on_server_closed), core);
    g_signal_connect(hub->connection, "message", G_CALLBACK(on_server_message), core);

    // register to server after connect to signalling servẻ  
    return;
}

void
connect_signalling_handler(SessionCore* core)
{
    Pipeline* pipe = session_core_get_pipeline(core);
    SignallingHub* hub = session_core_get_signalling_hub(core);
    GstElement* webrtcbin = pipeline_get_webrtc_bin(pipe);

    /* Add stun server */
    g_object_set(webrtcbin, "stun-server", 
       "stun://stun.thinkmay.net:3478", NULL);

    g_signal_emit_by_name (webrtcbin, "add-turn-server", hub->turn, NULL);


    /* This is the gstwebrtc entry point where we create the offer and so on. It
     * will be called when the pipeline goes to PLAYING. */
    g_signal_connect(webrtcbin, "on-negotiation-needed",
        G_CALLBACK(on_negotiation_needed), core);
    g_signal_connect(webrtcbin, "on-ice-candidate",
        G_CALLBACK(send_ice_candidate_message), core);
    g_signal_connect(webrtcbin, "notify::ice-gathering-state",
        G_CALLBACK(on_ice_gathering_state_notify), core);
}


void
signalling_hub_setup_turn_and_stun(Pipeline* pipeline,
                                  SignallingHub* hub) 
{
    GstElement* webrtcbin = pipeline_get_webrtc_bin(pipeline);
    g_object_set(webrtcbin, "turn-server",hub->turn,NULL);

    for (int i = 0; i < 5; i++)
    {
        if(strlen(hub->stuns[i]) > 0)
        {
            g_object_set(webrtcbin, "stun-server",hub->stuns[i],NULL);
            return;
        }
    }
}

