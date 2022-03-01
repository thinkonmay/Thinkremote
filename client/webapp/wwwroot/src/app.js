import { initGUI } from "./GUI.js";
import { AttachEvent, DetachEvent } from "./input.js";
import { SignallingConnect } from "./signalling.js";
import { WebrtcConnect } from "./webrtc.js";

export const getSignallingConnectionString = () => {
    return `${app.SignallingUrl}?token=${app.remoteToken}`;
}

/**
 * 
 * @param {string} message 
 */
export const setDebug = (message) => 
{
    console.log(message);
}

/**
 * 
 * @param {string} message 
 */
export const clientLog = (message) => 
{
    console.log(message);
}


export const reloadStream = () =>{
    DetachEvent();
    setTimeout(() => {
        window.location.reload()
    },2000);
}


window.onload = function () 
{
    initGUI();
    WebrtcConnect();
    SignallingConnect();
}