import { ShortcutOpcode } from "./enum.js";
import { requestKeyboardLock, updateMouseOffset } from "./input.js";

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
    },

    VideoElement: null,
    LoadingElement: null,
    fraction: 0
}


/**
 * 
 * enter full screen mode, all functional keywill be activated
 */
export function enterFullscreen() {
    GUI.VideoElement.parentElement.requestFullscreen();
}

/**
 * Handles incoming track event from peer connection.
 *
 * @param {Event} event - Track event: https://developer.mozilla.org/en-US/docs/Web/API/RTCTrackEvent
 */
export function onRemoteTrack(event) {
    if (GUI.VideoElement.srcObject !== event.streams[0]) {
        GUI.VideoElement.srcObject = event.streams[0];
        GUI.LoadingElement.innerHTML= "";
        console.log('Incoming stream');
    }
}

export const getVideoElement = () =>{
    return GUI.VideoElement;
}

/**
 * When fullscreen is entered, request keyboard and pointer lock.
 */
function 
onFullscreenChange() 
{
    requestKeyboardLock();   
    triggerEventByOpcode(ShortcutOpcode.FULLSCREEN);
    reset_keyboard();
}


/**
 * Captures display and video dimensions required for computing mouse pointer position.
 * app should be fired whenever the window size changes.
 */
export function
windowCalculate() 
{
    /**
     * size of video element (included its border) on client screen
     * (displayed video size)
     */
    GUI.Screen.clientWidth = GUI.VideoElement.offsetWidth;
    GUI.Screen.clientHeight = GUI.VideoElement.offsetHeight;

    /**
     * actual video width and height of incoming stream 
     * (registered in session initialize step)
     */
    GUI.Screen.StreamWidth =  GUI.VideoElement.videoWidth;
    GUI.Screen.StreamHeight = GUI.VideoElement.videoHeight;

    /**
     * fraction between displayed video size and incoming stream framesize
     * (both width and height fraction is acceptable)
     */
    GUI.fraction = Math.min
        (GUI.Screen.clientWidth  / GUI.Screen.StreamWidth, 
         GUI.Screen.clientHeight / GUI.Screen.StreamHeight);



    const vpWidth =  GUI.Screen.StreamWidth  * GUI.Screen.fraction;
    const vpHeight = GUI.Screen.StreamHeight * GUI.Screen.fraction;



    /**
     * reposition mouse after screen resolution has been changed
     */
    var Mouse = 
    {
        /**
         * relation between frame size and actual window size
         */
        mouseMultiX:     GUI.Screen.StreamWidth / vpWidth,
        mouseMultiY:     GUI.Screen.StreamHeight / vpHeight,

        /**
         * 
         */
        mouseOffsetX:    Math.max((GUI.Screen.clientWidth  - vpWidth) / 2.0, 0),
        mouseOffsetY:    Math.max((GUI.Screen.clientHeight - vpHeight) / 2.0, 0),


        /**
         * 
         */
        centerOffsetX:   (document.documentElement.clientWidth - GUI.VideoElement.offsetWidth) / 2.0,
        centerOffsetY:   (document.documentElement.clientHeight - GUI.VideoElement.offsetHeight) / 2.0,

        /**
         * 
         */
        scrollX:         window.scrollX,
        scrollY:         window.scrollY,

        /**
         * 
         */
        frameW:          GUI.Screen.StreamWidth,
        frameH:          GUI.Screen.StreamHeight,
    };

    updateMouseOffset(Mouse);
}


export const initGUI = () => {
    GUI.VideoElement = document.getElementById("stream");
    GUI.LoadingElement = document.getElementById("loading");
    GUI.VideoElement.parentElement.addEventListener('fullscreenchange', onFullscreenChange);
}