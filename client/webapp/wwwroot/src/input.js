import { sendHIDMessage } from "./datachannel.js";
import { JavaScriptOpcode, ShortcutOpcode } from "./enum.js";
import { enterFullscreen, getVideoElement, windowCalculate } from "./GUI.js";

var CaptureInput = 
{
    Mouse: 
    {
        /**
         * relation between frame size and actual window size
         * (used to determine relation between client mouse and its position on slave screen)
         */
        mouseMultiX: 0,
        mouseMultiY: 0,

        /**
         * 
         */
        mouseOffsetX: 0,
        mouseOffsetY: 0,

        /**
        *
        */
        centerOffsetX: 0,
        centerOffsetY: 0,

        /*
        *
        */
        scrollX: 0,
        scrollY: 0,

        /*
        *
        */
        frameW:0,
        frameH:0,
    },

    relativeMouse:false,

    shortcuts: [],
    EventListeners: [],
}

export const new_shortcut = (Opcode,key_list) => {
    var shortcutTriggered = new Event('trigger');
    CaptureInput.shortcuts.push({
        Opcode: Opcode,
        key_list: key_list,
        event: shortcutTriggered
    }); 
}

export const triggerEventByOpcode = (Opcode) => {
    CaptureInput.shortcuts.forEach((element) =>{
        if(element.Opcode === Opcode)
        {
            element.dispatchEvent(element.event);
        }
    })
}




/**
 * Handle mouse up event and send to slave device
 * @param {Mouse up event} event 
 */
function 
mouseButtonUp(event) 
{
    var INPUT =
    {
        Opcode:   JavaScriptOpcode.MOUSE_UP,
        button:   event.button,
    };

    sendHIDMessage(JSON.stringify(INPUT));
    event.preventDefault();
}

/**
 * Handle mouse down event and send to slave device
 * @param {Mouse event} event 
 */
function 
mouseButtonDown(event) 
{
    if(CaptureInput.relativeMouse)
    {        
        var INPUT =
        {
            Opcode:   JavaScriptOpcode.MOUSE_DOWN,
            button:   event.button,
        }
    
        sendHIDMessage(JSON.stringify(INPUT));
    }
    else
    {
        var mousePosition_X = clientToServerX(event.clientX);
        var mousePosition_Y = clientToServerY(event.clientY);
        
        var INPUT =
        {
            Opcode:   JavaScriptOpcode.MOUSE_DOWN,
            button:   event.button,
            dX:       mousePosition_X,
            dY:       mousePosition_Y
        }
    
        sendHIDMessage(JSON.stringify(INPUT));
    }
    event.preventDefault();
}

/**
 * Handle mouse movement and send to slave
 * @param {Mouse movement} event 
 */
function 
mouseButtonMovement(event) 
{
    var mousePosition_X; 
    var mousePosition_Y;

    if(CaptureInput.relativeMouse)
    {
        mousePosition_X = event.movementX;
        mousePosition_Y = event.movementY;
    }else
    {
        mousePosition_X = clientToServerX(event.clientX);
        mousePosition_Y = clientToServerY(event.clientY);
    }

    var INPUT =
    {
        "Opcode":   JavaScriptOpcode.MOUSE_MOVE,
        "dX":       mousePosition_X,
        "dY":       mousePosition_Y,
    }
    sendHIDMessage(JSON.stringify(INPUT));
}

/**
 * handle mouse wheel and send to slave
 * @param {Mouse wheel event} event 
 */
function 
mouseWheel(event)
{
    var INPUT =
    {
        "Opcode":   JavaScriptOpcode.MOUSE_WHEEL,
        "WheeldY":  event.deltaY
    }
    sendHIDMessage(JSON.stringify(INPUT));
}


///handle context menu by disable it
function 
contextMenu(event) 
{ 
    event.preventDefault();
}

function keyup(event) 
{  
    var Keyboard =
    {
        Opcode:     JavaScriptOpcode.KEYUP,
        wVk:        event.code,
    }

    sendHIDMessage(JSON.stringify(Keyboard));

    // disable problematic browser shortcuts
    if ((event.code === 'F5' && event.ctrlKey)||
        (event.code === 'Tab')||
        (event.code === 'KeyI' && event.ctrlKey && event.shiftKey)||
        (event.code === 'KeyW' && event.ctrlKey)||
        (event.code === 'F11') )
    {
        event.preventDefault();
    } 
}


function 
keydown(event) 
{
    if (event.code === 'KeyP' && event.ctrlKey && event.shiftKey) {
        if(!document.pointerLockElement)
        {
            var VideoElement = getVideoElement();
            VideoElement.requestPointerLock();
            event.preventDefault();
            return;
        }
        else
        {
            document.exitPointerLock();
            event.preventDefault();
            return;
        }
    }

    // capture menu hotkey
    if (event.code === 'KeyF' && event.ctrlKey && event.shiftKey) 
    {
        if (document.fullscreenElement === null) 
        {
            enterFullscreen();
            event.preventDefault();
            return;
        }
    }
    var Keyboard =
    {
        Opcode   : JavaScriptOpcode.KEYDOWN,
        wVk      : event.code,
    }

    sendHIDMessage(JSON.stringify(Keyboard));

    // disable problematic browser shortcuts
    if ((event.code === 'F5' && event.ctrlKey) ||
        (event.code === 'Tab') ||
        (event.code === 'KeyI' && event.ctrlKey && event.shiftKey) ||
        (event.code === 'KeyW' && event.ctrlKey) ||
        (event.code === 'F11')) {
        event.preventDefault();
        return;
    }
}



/**
 * Translates pointer position X based on current window math.
 * @param {Integer} clientX
 */
function 
clientToServerX(clientX) 
{
    let serverX = Math.round
    (
        (clientX - CaptureInput.Mouse.mouseOffsetX - CaptureInput.Mouse.centerOffsetX + CaptureInput.Mouse.scrollX) * CaptureInput.Mouse.mouseMultiX
    );

    if (serverX === CaptureInput.Mouse.frameW - 1) serverX = CaptureInput.Mouse.frameW;
    if (serverX > CaptureInput.Mouse.frameW) serverX = CaptureInput.Mouse.frameW;
    if (serverX < 0) serverX = 0;

    return Math.round(serverX);
}

/**
 * Translates pointer position Y based on current window math.
 * @param {Integer} clientY
 */
function 
clientToServerY(clientY) 
{
    /**
     * mouse position on slave device is calculated by 
     */
    let serverY = Math.round(
        (clientY - CaptureInput.Mouse.mouseOffsetY - CaptureInput.Mouse.centerOffsetY + CaptureInput.Mouse.scrollY)
            * CaptureInput.Mouse.mouseMultiY);

    if (serverY === CaptureInput.Mouse.frameH - 1) serverY = CaptureInput.Mouse.frameH;
    if (serverY > CaptureInput.Mouse.frameH) serverY = CaptureInput.Mouse.frameH;
    if (serverY < 0) serverY = 0;

    return  Math.round(serverY);
}





function
mouseLeaveEvent(event)
{
    triggerEventByOpcode(ShortcutOpcode.RESET_KEY);
}


/**
 * Attaches input event handles to docuemnt, window and element.
 */
export function 
AttachEvent() 
{
    /**
     * determine screen parameter before connect HID event handler
     */
    windowCalculate();
    var VideoElement = getVideoElement();

    /**
     * video event
     */
    CaptureInput.EventListeners.push(addListener(VideoElement, 'contextmenu', contextMenu, null)); ///disable content menu key on remote control

    /**
     * mouse event
     */
    CaptureInput.EventListeners.push(addListener(VideoElement, 'wheel', mouseWheel, null));
    CaptureInput.EventListeners.push(addListener(VideoElement, 'mousemove', mouseButtonMovement, null));
    CaptureInput.EventListeners.push(addListener(VideoElement, 'mousedown', mouseButtonDown, null));
    CaptureInput.EventListeners.push(addListener(VideoElement, 'mouseup', mouseButtonUp, null));

    /**
     * mouse lock event
     */
    CaptureInput.EventListeners.push(addListener(VideoElement, 'mouseleave', mouseLeaveEvent, null));
    CaptureInput.EventListeners.push(addListener(document,     'pointerlockchange', pointerLock, null));
    
    /**
     * keyboard event
     */
    CaptureInput.EventListeners.push(addListener(window, 'keydown', keydown, null));
    CaptureInput.EventListeners.push(addListener(window, 'keyup', keyup, null));

    /**
     * scroll event
     */
    CaptureInput.EventListeners.push(addListener(window, 'scroll', () => {
        CaptureInput.Mouse.scrollX = window.scrollX;
        CaptureInput.Mouse.scrollY = window.scrollY;
    }));

}


/**
 * Sends WebRTC app command to toggle display of the remote mouse pointer.
 */
function pointerLock() {

    if (document.pointerLockElement) 
        CaptureInput.relativeMouse = true;
    else 
        CaptureInput.relativeMouse = false;

    var INPUT =
    {
        "Opcode":JavaScriptOpcode.POINTER_LOCK,
        "Value":false
    }
    sendHIDMessage(JSON.stringify(INPUT));
}


export function 
DetachEvent() 
{
    removeListeners(CaptureInput.EventListeners);
    document.exitPointerLock();
    reset_keyboard();
}

/**
 * Request keyboard lock, must be in fullscreen mode to work.
 */
export function 
requestKeyboardLock() 
{
    /**
     * control key on window
     */
     const keys = [
        "AltLeft",
        "AltRight",
        "Tab",
        "Escape",
        "ContextMenu",
        "MetaLeft",
        "MetaRight"
    ];
    
    console.log("requesting keyboard lock");
    Navigator.keyboard.lock(keys)
    .then(
        () => {
            console.log("keyboard lock success");
        }
    ).catch(
        (e) => {
            console.log("keyboard lock failed: ", e);
        }
    )
}







/**
 * Helper function to keep track of attached event listeners.
 * @param {Object} obj
 * @param {string} name
 * @param {function} func
 * @param {Object} ctx
 */
function 
addListener(obj, name, func, ctx) 
{
    const newFunc = ctx ? func.bind(ctx) : func;
    obj.addEventListener(name, newFunc);
    return [obj, name, newFunc];
}

/**
 * Helper function to remove all attached event listeners.
 * @param {Array} listeners
 */
function 
removeListeners(listeners) 
{
    for (const listener of listeners)
        listener[0].removeEventListener(listener[1], listener[2]);
}


export function
updateMouseOffset(mouse)
{
    CaptureInput.Mouse = mouse;
}