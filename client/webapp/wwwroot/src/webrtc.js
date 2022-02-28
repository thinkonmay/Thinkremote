import { setDebug } from "./app";
import { onRemoteTrack } from "./GUI";
import { SignallingSend } from "./signalling.js"

var RemotePipeline = 
{
    RTCPeerConnection:  null,
    VideoElement: null,
    
    State: 'Disconnected',
}



/**
 * 
 * @param {*} ice 
 */
export function 
onIncomingICE(ice) {
    setDebug("[ICE RECEIVED] "+ice);
    var candidate = new RTCIceCandidate(ice);
    RemotePipeline.RTCPeerConnection.addIceCandidate(candidate)
    .catch((error) =>
    {
        setDebug(error);
    });
}


/**
 * Handles incoming SDP from signalling server.
 * Sets the remote description on the peer connection,
 * creates an answer with a local description and sends that to the peer.
 *
 * @param {RTCSessionDescription} sdp
 */
export function 
onIncomingSDP(sdp) 
{
    setDebug("[SDP RECEIVED] "+sdp);
    if (sdp.type != "offer")
        return;

    RemotePipeline.State = "Got SDP offer";        

    RemotePipeline.RTCPeerConnection.setRemoteDescription(sdp).then(() => {
        RemotePipeline.RTCPeerConnection.createAnswer()
            .then(onLocalDescription).catch((error =>{
                setDebug(error);
            }));
        }).catch(error => {
        setDebug(error)
    });
}


/**
 * Handles local description creation from createAnswer.
 *
 * @param {RTCSessionDescription} local_sdp
 */
export function 
onLocalDescription(desc) {
    RemotePipeline.RTCPeerConnection.setLocalDescription(desc).then(function() {
        sdp = {'sdp': app.Webrtc.localDescription}
    
        SignallingSend("OFFER_SDP",JSON.stringify(sdp));
    });
}






    


/**
 * Control data channel has been estalished, 
 * start report stream stats to slave
 * @param {Event} event 
 */
function 
onControlDataChannel(event)
{
    app.ControlDC = event.channel;
    app.ControlDC.onmessage = (event =>{
        if(event.data == "ping") {
            app.ControlDC.send("ping");
        }
    });
}

function
onHidDataChannel(event)
{
    app.HidDC = event.channel;
    connectionDone();
}

function
ondatachannel(event)
{
    if(event.channel.label === "HID"){
        onHidDataChannel(event);
    }else if(event.channel.label === "Control"){
        onControlDataChannel(event);
    }
}


export function
onICECandidates(event)
{
    if (event.candidate == null) 
    {
        console.log("ICE Candidate was null, done");
        return;
    }

    SignallingSend("OFFER_ICE",JSON.stringify({'ice': event.candidate}));
}

/**
 * Initiate connection to signalling server. 
 * invoke after request sdp signal has been replied
 */
export function 
WebrtcConnect() 
{
    RemotePipeline.State             = null;
    RemotePipeline.RTCPeerConnection = null;

    var config = app.RTPconfig;
    RemotePipeline.RTCPeerConnection = new RTCPeerConnection(config);

    RemotePipeline.RTCPeerConnection.ondatachannel =  ondatachannel;    
    RemotePipeline.RTCPeerConnection.ontrack =        onRemoteTrack;
    RemotePipeline.RTCPeerConnection.onicecandidate = onICECandidates;
}
