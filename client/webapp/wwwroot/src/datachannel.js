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