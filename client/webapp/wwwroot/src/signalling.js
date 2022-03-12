/**
 * Fired whenever the signalling websocket is opened.
 * Sends the peer id to the signalling server.
 */
function    
onServerOpen(event)
{
    app.signalling_state = 'connected';
}


/**
 * send messsage to signalling server
 * @param {string} request_type 
 * @param {any} content 
 */
function   
SignallingSend(request_type, content)
{
    var json_message = {"RequestType":request_type,
                        "Content":content}

    app.Websocket.send(JSON.stringify(json_message));
}

/**
 * Fired whenever the signalling websocket emits and error.
 * Reconnects after 3 seconds.
 */
function    
onServerError() 
{
    app.signalling_state = 'disconnected';
    if (app.Websocket.readyState === app.Websocket.CLOSED) {
        setTimeout(() => {
            app.connectServer();
        }, 3000);
    }
}


/**
 * handle message from signalling server during connection handshake
 * @param {Event} event 
 * @returns 
 */
function   
onServerMessage(event) 
{
    try {
        if(event.data === "ping"){
            app.setDebug("ping signalling server successful")
            return;
        }
        var message_json = JSON.parse(event.data);
    } catch (e) {
        if (e instanceof SyntaxError) {
            app.setDebug("Error parsing incoming JSON: " + event.data);
        } else {
            app.setDebug("Unknown error parsing response: " + event.data);
        }
        return;
    }




    /**
     * initialize webrtc connection after receive sdp offer
     */
    if(app.Webrtc == null)
    {
        WebrtcConnect();
    }

    /**
     * webrtc instance has been established, now come to negotiation
     */
    if(message_json.RequestType === "OFFER_SDP")
    {
        app.setDebug("[SDP RECEIVED]"+JSON.stringify(message_json.Content));
        onIncomingSDP(JSON.parse(message_json.Content).sdp);
    }
    else if(message_json.RequestType === "OFFER_ICE")
    {
        app.setDebug("[ICE RECEIVED]"+message_json.Content);
        onIncomingICE(JSON.parse(message_json.Content).ice);
    }   
    else
    {
        /**
         * emit error if request type is not either OFFER ICE or SDP
         */
        app.setError("unknown message", event.data);
    } 
    
}

/**
 * Fired whenever the signalling websocket is closed.
 * Reconnects after 1 second.
 */
function    
onServerClose() {
    app.signalling_state = 'disconnected';
}

/**
 * Initiates the connection to the signalling server.
 */
function
SignallingConnect() 
{
    app.signalling_state = 'connecting';
    app.setStatus("Connecting to server.");

    app.Websocket = new WebSocket(app.SignallingUrl+"?token="+app.remoteToken);

    // Bind event handlers.
    app.Websocket.addEventListener('open', onServerOpen);
    app.Websocket.addEventListener('error', onServerError);
    app.Websocket.addEventListener('message', onServerMessage);
    app.Websocket.addEventListener('close', onServerClose);
}


