
#include <remote-app-signalling.h>
#include <remote-app.h>
#include <remote-app-pipeline.h>
#include <remote-app-type.h>

#include <signalling-message.h>
#include <development.h>
#include <string-manipulate.h>

#include <gst/gst.h>
#include <glib-2.0/glib.h>
#include <remote-app-type.h>
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
     * websocket connection to connect with signalling server
     */
    SoupWebsocketConnection* connection;

    /**
     * @brief 
     * soup session for connection witb signalling server
     */
    SoupSession* session;

    /**
     * @brief 
     * signalling server url
     */
	gchar signalling_server[200];

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


/**
 * @brief 
 * handle server connected signall
 * @param session 
 * @param res 
 * @param core 
 */
void                on_server_connected             (SoupSession* session,
                                                    GAsyncResult* res,
                                                    RemoteApp* core);

SignallingHub*
signalling_hub_initialize(RemoteApp* core)
{
    SignallingHub* hub = malloc(sizeof(SignallingHub));
    memset(hub,0,sizeof(SignallingHub));
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
    if(!turn)
    {
        if(DEVELOPMENT_ENVIRONMENT) 
        {
            g_printerr("Fail to get turn server, setting default value");
        }
        turn = DEFAULT_TURN;
    }
    else
    {
		g_print("starting remote session with turn server\n");
		g_print(turn);
		g_print("\n");
    }
    memcpy(hub->remote_token, remote_token,strlen(remote_token));
    memcpy(hub->signalling_server, url,strlen(url));
    memcpy(hub->turn, turn,strlen(turn));
    if(!stun_array)
        return;

    json_array_foreach_element(stun_array,
        (JsonArrayForeach)handle_stun_list,(gpointer)hub);
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

    soup_websocket_connection_send_text(signalling->connection,buffer);
    g_free(buffer);
}


/**
 * @brief 
 * send ice candidate to worker node
 * @param G_GNUC_UNUSED 
 * @param mlineindex 
 * @param candidate 
 * @param G_GNUC_UNUSED 
 */
static void
send_ice_candidate_message(GstElement* webrtc G_GNUC_UNUSED,
                            guint mlineindex,
                            gchar* candidate,
                            RemoteApp* core G_GNUC_UNUSED)
{
    gchar* text;
    JsonObject* ice, * msg;
    SignallingHub* hub = remote_app_get_signalling_hub(core);

    ice = json_object_new();
    json_object_set_string_member(ice, "candidate", candidate);
    json_object_set_int_member(ice, "sdpMLineIndex", mlineindex);
    msg = json_object_new();
    json_object_set_object_member(msg, "ice", ice);
    text = get_string_from_json_object(msg);
    json_object_unref(msg);

    send_message_to_signalling_server(hub,OFFER_ICE,text);
    g_free(text);
}


/**
 * @brief 
 * send session description to worker node
 * @param core 
 * @param desc 
 */
static void
send_sdp_to_peer(RemoteApp* core,
    GstWebRTCSessionDescription* desc)
{
    gchar* text;
    JsonObject* msg, * sdp;

    SignallingHub* hub = remote_app_get_signalling_hub(core);

    text = gst_sdp_message_as_text(desc->sdp);
    sdp = json_object_new();

    if (desc->type == GST_WEBRTC_SDP_TYPE_OFFER) 
    {
        json_object_set_string_member(sdp, "type", "offer");
    }
    else if (desc->type == GST_WEBRTC_SDP_TYPE_ANSWER) 
    {
        json_object_set_string_member(sdp, "type", "answer");
    }
    else 
    {
        g_assert_not_reached();
    }

    json_object_set_string_member(sdp, "sdp", text);

    msg = json_object_new();
    json_object_set_object_member(msg, "sdp", sdp);
    text = get_string_from_json_object(msg);
    json_object_unref(msg);

    send_message_to_signalling_server(hub,OFFER_SDP,text);
    g_free(text);
}


/**
 * @brief 
 * Offer created by our pipeline, to be sent to the peer 
 * @param promise 
 * @param core 
 * @return * Offer, 
 */
void
on_offer_created( GstPromise* promise, 
                  RemoteApp* core)
{
    GstWebRTCSessionDescription* offer = NULL;
    const GstStructure* reply;

    Pipeline* pipe = remote_app_get_pipeline(core);
    SignallingHub* hub = remote_app_get_signalling_hub(core);


    g_assert_cmphex(gst_promise_wait(promise), == , GST_PROMISE_RESULT_REPLIED);

    reply = gst_promise_get_reply(promise);
    gst_structure_get(reply, "offer",
        GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &offer, NULL);
    gst_promise_unref(promise);

    promise = gst_promise_new();
    g_signal_emit_by_name(pipeline_get_webrtc_bin(pipe),
        "set-local-description", offer, promise);

    gst_promise_interrupt(promise);

    /* Send offer to peer */
    send_sdp_to_peer(core,offer);
    gst_webrtc_session_description_free(offer);
}

/**
 * @brief 
 * on negotiation needed signal, emited by webrtcbin
 * @param element 
 * @param core 
 */
static void
on_negotiation_needed(GstElement* element, 
                      RemoteApp* core)
{
    Pipeline* pipe = remote_app_get_pipeline(core);
    SignallingHub* signalling = remote_app_get_signalling_hub(core);


    GstPromise* promise =
    gst_promise_new_with_change_func(on_offer_created, core, NULL);

    g_signal_emit_by_name(pipeline_get_webrtc_bin(pipe),
        "create-offer", NULL, promise);
}


/**
 * @brief 
 * on ice gathering state notify signal, emit by webrtcbin
 * @param webrtcbin 
 * @param pspec 
 * @param user_data 
 */
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


/**
 * @brief 
 * handle close event from websocket 
 * @param G_GNUC_UNUSED 
 * @param G_GNUC_UNUSED 
 */
void
on_server_closed(SoupWebsocketConnection* conn G_GNUC_UNUSED,
    RemoteApp* core G_GNUC_UNUSED)
{
    SignallingHub* hub = remote_app_get_signalling_hub(core);
    hub->connection = NULL;
    hub->session = NULL;
    remote_app_finalize(core,NULL);
}



/**
 * @brief 
 * answer sdp offer
 * @param promise 
 * @param core 
 */
static void
on_answer_created(GstPromise* promise,
    RemoteApp* core)
{
    GstWebRTCSessionDescription* answer = NULL;
    const GstStructure* reply;

    Pipeline* pipe = remote_app_get_pipeline(core);
    SignallingHub* hub = remote_app_get_signalling_hub(core);


    g_assert_cmphex(gst_promise_wait(promise), == , GST_PROMISE_RESULT_REPLIED);
    reply = gst_promise_get_reply(promise);
    gst_structure_get(reply, "answer",
        GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &answer, NULL);
    gst_promise_unref(promise);

    promise = gst_promise_new();

    g_signal_emit_by_name( pipeline_get_webrtc_bin(pipe),
        "set-local-description", answer, promise);

    gst_promise_interrupt(promise);

    /* Send answer to peer */
    send_sdp_to_peer(core,answer);
    gst_webrtc_session_description_free(answer);
}

/**
 * @brief 
 * 
 * @param promise 
 * @param core 
 */
static void
on_offer_set(GstPromise* promise,
            RemoteApp* core)
{
    Pipeline* pipe = remote_app_get_pipeline(core);
    GstElement* webrtc = pipeline_get_webrtc_bin(pipe);

    gst_promise_unref(promise);
    promise = gst_promise_new_with_change_func(on_answer_created, 
        core, NULL);

    g_signal_emit_by_name(webrtc,
        "create-answer", NULL, promise);
}


/**
 * @brief 
 * handle sdp offer from worker node
 * @param core 
 * @param sdp 
 */
static void
on_offer_received(RemoteApp* core, 
                  GstSDPMessage* sdp)
{
    GstWebRTCSessionDescription* offer = NULL;
    GstPromise* promise;

    Pipeline* pipe = remote_app_get_pipeline(core);

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
signalling_connect(RemoteApp* core)
{
    SoupLogger* logger;
    SoupMessage* message;
    const char* https_aliases[] = { "wss", NULL };
    JsonObject* json_object;
    SignallingHub* hub = remote_app_get_signalling_hub(core);
    
    GString* string = g_string_new(hub->signalling_server);
    g_string_append(string,"?token=");
    g_string_append(string,hub->remote_token);
    gchar* url = g_string_free(string,FALSE);



    hub->session =
        soup_session_new_with_options(SOUP_SESSION_SSL_STRICT, TRUE,
            SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
            SOUP_SESSION_HTTPS_ALIASES, https_aliases, NULL);

    message = soup_message_new(SOUP_METHOD_GET, url);
    soup_session_websocket_connect_async(hub->session,
        message, NULL, NULL, NULL,
        (GAsyncReadyCallback)on_server_connected, core);
}



/**
 * @brief 
 * handle ice message from worker node
 * @param text 
 * @param core 
 */
static void
on_ice_exchange(gchar* text,RemoteApp* core)
{
    Pipeline* pipe = remote_app_get_pipeline(core);

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



/**
 * @brief 
 * handle sdp message from worker node
 * @param data 
 * @param core 
 */
static void
on_sdp_exchange(gchar* data, 
                RemoteApp* core)
{
    SignallingHub* hub = remote_app_get_signalling_hub(core);
    Pipeline* pipe = remote_app_get_pipeline(core);

    GError* error = NULL;
    JsonParser* parser = json_parser_new();
    JsonObject* object = get_json_object_from_string(data,&error,parser);
	if(!error == NULL || object == NULL) {remote_app_finalize(core,error);}

    gint ret;
    GstSDPMessage* sdp;
    const gchar* text;
    GstWebRTCSessionDescription* answer;

    JsonObject* child = json_object_get_object_member(object, "sdp");
    gchar* sdptype = json_object_get_string_member(child, "type");

    if (!json_object_has_member(child, "type"))
    {
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

/**
 * @brief 
 * handle message from signalling server
 * @param conn 
 * @param type 
 * @param message message string
 * @param core remteo app
 */
static void
on_server_message(SoupWebsocketConnection* conn,
    SoupWebsocketDataType type,
    GBytes* message,
    RemoteApp* core)
{
    Pipeline* pipe = remote_app_get_pipeline(core);
    gchar* text;

    switch (type) 
    {
        case SOUP_WEBSOCKET_DATA_BINARY:
        {
            return;
        } 
        case SOUP_WEBSOCKET_DATA_TEXT: 
        {
            gsize size;
            const char* data = g_bytes_get_data(message, &size);
            /* Convert to NULL-terminated string */
            text = g_strndup(data, size);
            break;
        }
    }



    GError* error = NULL;
    JsonParser* parser = json_parser_new();
    JsonObject* object = get_json_object_from_string(text,&error,parser);
	if(!error == NULL || object == NULL) {return;}

    gchar* RequestType =    json_object_get_string_member(object, "RequestType");
    gchar* Content =        json_object_get_string_member(object, "Content");
    if(DEVELOPMENT_ENVIRONMENT)
    {
        g_print(Content);
        g_print("\n");
    }

    if (!g_strcmp0(RequestType, "OFFER_SDP")) {
        on_sdp_exchange(Content, core);
    } else if (!g_strcmp0(RequestType, "OFFER_ICE")) {
        on_ice_exchange(Content, core);
    }
    g_free(text);
    g_object_unref(parser);
}


/**
 * @brief 
 * handle server connection done,
 * @param session 
 * @param res 
 * @param core 
 */
void
on_server_connected(SoupSession* session,
    GAsyncResult* res,
    RemoteApp* core)
{
    GError* error = NULL;
    SignallingHub* hub = remote_app_get_signalling_hub(core);

    
    hub->connection = soup_session_websocket_connect_finish(session, res, &error);
    if (!error == NULL || hub->connection == NULL) 
    {
        remote_app_finalize(core, error);
    }

    g_signal_connect(hub->connection, "closed", G_CALLBACK(on_server_closed), core);
    g_signal_connect(hub->connection, "message", G_CALLBACK(on_server_message), core);
    return;
}


gboolean
signalling_close(SignallingHub* hub)
{
    if (hub->connection)
    {
        if (soup_websocket_connection_get_state(hub->connection) == SOUP_WEBSOCKET_STATE_OPEN)
            soup_websocket_connection_close(hub->connection, 1000, "");
        else
            g_object_unref(hub->connection);
    }
}

void
connect_signalling_handler(RemoteApp* core)
{
    Pipeline* pipe = remote_app_get_pipeline(core);
    SignallingHub* hub = remote_app_get_signalling_hub(core);
    GstElement* webrtcbin = pipeline_get_webrtc_bin(pipe);

    /* Add turn server */
    g_object_set(webrtcbin, "turn-server", hub->turn, NULL);
    g_object_set(webrtcbin, "stun-server", hub->stuns[0], NULL);

    /* This is the gstwebrtc entry point where we create the offer and so on. It
     * will be called when the pipeline goes to PLAYING. */
    g_signal_connect(webrtcbin, "on-negotiation-needed",
        G_CALLBACK(on_negotiation_needed), core);
    g_signal_connect(webrtcbin, "on-ice-candidate",
        G_CALLBACK(send_ice_candidate_message), core);
    g_signal_connect(webrtcbin, "notify::ice-gathering-state",
        G_CALLBACK(on_ice_gathering_state_notify), core);
}


