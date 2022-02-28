
var GUI = 
{
    /**
    * default Value of QoE metric, fetch from server
    */
    Screen:
    {
        /*
        * frame resolution used to transport to client
        */
        StreamWidth: 0,
        StreamHeight: 0,


        /*
        * client resolution display on client screen
        */
        ClientWidth: 0,
        ClientHeight: 0,

        fraction: 0
    },

    VideoElement: null,
}


///enter full screen mode, all functional keywill be activated
export function enterFullscreen() {
    // Request full screen mode.
    this.VideoElement.parentElement.requestFullscreen();
}

/**
 * Handles incoming track event from peer connection.
 *
 * @param {Event} event - Track event: https://developer.mozilla.org/en-US/docs/Web/API/RTCTrackEvent
 */
export function onRemoteTrack(event) {
    if (app.VideoElement.srcObject !== event.streams[0]) {
        app.VideoElement.srcObject = event.streams[0];
        console.log('Incoming stream');
    }
}