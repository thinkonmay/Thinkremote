import * as APP from "./app.js"
import { setDebug } from "./app.js";
import { onIncomingICE, onIncomingSDP, WebrtcConnect } from "./webrtc.js";

var SignallingHub = 
{
    state: "Disconnected",
    WebSocketConnection: null,
}


/**
 * Fired whenever the signalling websocket is opened.
 * Sends the peer id to the signalling server.
 */
function    
onServerOpen(event)
{
    SignallingHub.state = 'connected';
}


/**
 * send messsage to signalling server
 * @param {string} request_type 
 * @param {any} content 
 */
export function
SignallingSend(request_type, content)
{
    setDebug(`${request_type} : ${content}`);

    var json_message = JSON.stringify(
        {RequestType:       request_type,
         Content:           content});

    SignallingHub.WebSocketConnection.send(json_message);
}

/**
 * Fired whenever the signalling websocket emits and error.
 * Reconnects after 3 seconds.
 */
function    
onServerError() 
{
    SignallingHub.state = 'disconnected';
    APP.reloadStream();
}


/**
 * handle message from signalling server during connection handshake
 * @param {Event} event 
 * @returns 
 */
function   
onServerMessage(event) 
{
    if(event.data === "ping") 
        return;

    var message_json = JSON.parse(event.data);

    if(message_json.RequestType === "OFFER_SDP")
    {
        WebrtcConnect();
        onIncomingSDP(JSON.parse(message_json.Content).sdp);
    }
    else if(message_json.RequestType === "OFFER_ICE")
        onIncomingICE(JSON.parse(message_json.Content).ice);
}


/**
 * Initiates the connection to the signalling server.
 */
export function
SignallingConnect() 
{
    SignallingHub.state = 'connecting';
    SignallingHub.WebSocketConnection = new WebSocket(APP.getSignallingConnectionString());

    SignallingHub.WebSocketConnection.addEventListener('open', onServerOpen);
    SignallingHub.WebSocketConnection.addEventListener('error', onServerError);
    SignallingHub.WebSocketConnection.addEventListener('message', onServerMessage);
    SignallingHub.WebSocketConnection.addEventListener('close', onServerError);
}