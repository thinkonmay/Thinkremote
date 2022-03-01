import { setDebug } from "./app.js";
import { ondatachannel } from "./datachannel.js";
import { onRemoteTrack } from "./GUI.js";
import { startCollectingStat } from "./quality-track.js";
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
function 
onLocalDescription(desc) {
    RemotePipeline.RTCPeerConnection.setLocalDescription(desc).then(function() {
        var sdp = {'sdp': RemotePipeline.RTCPeerConnection.localDescription}
        SignallingSend("OFFER_SDP",JSON.stringify(sdp));
    });
}



function
onICECandidates(event)
{
    if (event.candidate == null) 
    {
        console.log("ICE Candidate was null, done");
        return;
    }

    SignallingSend("OFFER_ICE",JSON.stringify({
        'ice': event.candidate
    }));
}

/**
 * Initiate connection to signalling server. 
 * invoke after request sdp signal has been replied
 */
export function 
WebrtcConnect(RTCconfig) 
{
    RemotePipeline.State             = null;
    RemotePipeline.RTCPeerConnection = null;

    RemotePipeline.RTCPeerConnection = new RTCPeerConnection(RTCconfig);

    RemotePipeline.RTCPeerConnection.ondatachannel =  ondatachannel;    
    RemotePipeline.RTCPeerConnection.ontrack =        onRemoteTrack;
    RemotePipeline.RTCPeerConnection.onicecandidate = onICECandidates;

    startCollectingStat();
}

export const
getRTCConnection = () => 
{
    return RemotePipeline.RTCPeerConnection;
}
