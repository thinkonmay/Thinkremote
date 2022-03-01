var WebRTCDataChannel=
{
    HID: null,
    Control: null,
}


export const sendHIDMessage = (message) => {
    WebRTCDataChannel.HID.send(message);
}

export const sendControlMessage = (message) => {
    WebRTCDataChannel.Control.send(message);
}

/**
 * Control data channel has been estalished, 
 * start report stream stats to slave
 * @param {Event} event 
 */
function 
onControlDataChannel(event)
{
    WebRTCDataChannel.Control = event.channel;
    WebRTCDataChannel.Control.onmessage = (event =>{
        if(event.data == "ping") {
            WebRTCDataChannel.Control.send("ping");
        }
    });
}

/**
 * 
 * @param {Event} event 
 */
function
onHidDataChannel(event)
{
    WebRTCDataChannel.HID = event.channel;
}

export function
ondatachannel(event)
{
    if(event.channel.label === "HID")
        onHidDataChannel(event);

    if(event.channel.label === "Control")
        onControlDataChannel(event);
}