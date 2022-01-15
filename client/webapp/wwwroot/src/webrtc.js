/**
 * 
 * @param {*} ice 
 */
function onIncomingICE(ice) {
    var candidate = new RTCIceCandidate(ice);
    app.Webrtc.addIceCandidate(candidate).catch(app.setError);
}









/**
 * Handles incoming SDP from signalling server.
 * Sets the remote description on the peer connection,
 * creates an answer with a local description and sends that to the peer.
 *
 * @param {RTCSessionDescription} sdp
 */
function onIncomingSDP(sdp) {
    app.Webrtc.setRemoteDescription(sdp).then(() => {
        app.setStatus("Remote SDP set");
        if (sdp.type != "offer")
            return;
        app.setStatus("Got SDP offer");        
        app.Webrtc.createAnswer()
            .then(onLocalDescription).catch(app.setError);
        
    }).catch(app.setError);
}


/**
 * Handles local description creation from createAnswer.
 *
 * @param {RTCSessionDescription} local_sdp
 */
function onLocalDescription(desc) {
    app.Webrtc.setLocalDescription(desc).then(function() {
        app.setStatus("Sending SDP " + desc.type);
        sdp = {'sdp': app.Webrtc.localDescription}
    
    console.log("[Send SDP]: " + JSON.stringify(desc));
    SignallingSend("OFFER_SDP",JSON.stringify(sdp));
    });
}




/**
 * Handles incoming track event from peer connection.
 *
 * @param {Event} event - Track event: https://developer.mozilla.org/en-US/docs/Web/API/RTCTrackEvent
 */
 function onRemoteTrack(event) {
    if (app.VideoElement.srcObject !== event.streams[0]) {
        console.log('Incoming stream');
        app.VideoElement.srcObject = event.streams[0];
    }
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


function
onICECandidates(event)
{
    // We have a candidate, send it to the remote party with the
    // same uuid
    if (event.candidate == null) {
        console.log("ICE Candidate was null, done");
        document.getElementById("loading").innerHTML = " ";
        return;
    }
    app.setDebug("OFFER_ICE" + JSON.stringify({'ice': event.candidate}));
    SignallingSend("OFFER_ICE",JSON.stringify({'ice': event.candidate}));
}

/**
 * Initiate connection to signalling server. 
 * invoke after request sdp signal has been replied
 */
function 
WebrtcConnect() 
{
    console.log('Creating RTCPeerConnection');

    var config = app.RTPconfig;
    app.Webrtc = new RTCPeerConnection(config);
    app.Webrtc.ondatachannel = ondatachannel;    
    app.Webrtc.ontrack = onRemoteTrack;
    app.Webrtc.onicecandidate = onICECandidates;
}
