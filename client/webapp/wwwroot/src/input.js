/**
 * attach event handler when Hid channel is connected
 * @param {Event} event 
 */
function 
connectionDone()  
{
    AttachEvent();
}


/**
 * Handle mouse up event and send to slave device
 * @param {Mouse up event} event 
 */
function 
mouseButtonUp(event) 
{
    if(app.Mouse.relativeMouse)
    {        
        var INPUT =
        {
            "Opcode":   HidOpcode.MOUSE_UP,
            "button":   event.button,
        }
    
        app.HidDC.send(JSON.stringify(INPUT));
    }else
    {
        var mousePosition_X = clientToServerX(event.clientX);
        var mousePosition_Y = clientToServerY(event.clientY);

        var INPUT =
        {
            "Opcode":   HidOpcode.MOUSE_UP,
            "button":   event.button,
            "dX":       mousePosition_X,
            "dY":       mousePosition_Y,
        }
    
        app.HidDC.send(JSON.stringify(INPUT));
    }
}

/**
 * Handle mouse down event and send to slave device
 * @param {Mouse event} event 
 */
function 
mouseButtonDown(event) 
{
    if(app.Mouse.relativeMouse)
    {        
        var INPUT =
        {
            "Opcode":   HidOpcode.MOUSE_DOWN,
            "button":   event.button,
        }
    
        app.HidDC.send(JSON.stringify(INPUT));
    }
    else
    {
        var mousePosition_X = clientToServerX(event.clientX);
        var mousePosition_Y = clientToServerY(event.clientY);
        
        var INPUT =
        {
            "Opcode":   HidOpcode.MOUSE_DOWN,
            "button":   event.button,
            "dX":       mousePosition_X,
            "dY":       mousePosition_Y
        }
    
        app.HidDC.send(JSON.stringify(INPUT));
    }
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
    if(app.Mouse.relativeMouse)
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
        "Opcode":   HidOpcode.MOUSE_MOVE,
        "dX":       mousePosition_X,
        "dY":       mousePosition_Y,
    }
    app.HidDC.send(JSON.stringify(INPUT));
}

/**
 * handle mouse wheel and send to slave
 * @param {Mouse wheel event} event 
 */
function 
mouseWheel(event)
{
    var mousePosition_X = clientToServerX(event.clientX);
    var mousePosition_Y = clientToServerY(event.clientY);

    var INPUT =
    {
        "Opcode":   HidOpcode.MOUSE_WHEEL,
        "dX":       mousePosition_X,
        "dY":       mousePosition_Y,
        "WheeldY":  event.deltaY
    }

    app.HidDC.send(JSON.stringify(INPUT));
}

function 
reset_mouse()
{
    var mousePosition_X = clientToServerX(0);
    var mousePosition_Y = clientToServerY(0);              

    var array = [0,1,2]; 

    array.forEach(element => {
        var INPUT =
        {
            "Opcode":HidOpcode.MOUSE_UP,
            "button":element,
            "dX":mousePosition_X,
            "dY":mousePosition_Y,
        }

        app.HidDC.send(JSON.stringify(INPUT));
    });
}

function 
reset_keyboard()
{
    var array = [
            "ControlLeft",
            "ShiftLeft",
            "AltLeft",
            "Home",
            "MetaLeft",
            "KeyF",
            "KeyM",
            "Escape"
        ]; 
    array.forEach(element => {
        var INPUT = 
        {
            "Opcode":HidOpcode.KEYUP,
            "wVk": element,
        }
        app.HidDC.send(JSON.stringify(INPUT));
    });
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
        "Opcode":HidOpcode.KEYUP,
        "wVk":event.code,
    }

    app.HidDC.send(JSON.stringify(Keyboard));

    // disable problematic browser shortcuts
    if (event.code === 'F5' && event.ctrlKey ||
        event.code === 'Tab' ||
        event.code === 'KeyI' && event.ctrlKey && event.shiftKey ||
        event.code === 'KeyW' && event.ctrlKey ||
        event.code === 'F11') {
        event.preventDefault();
        return;
    }


}


function 
keydown(event) 
{
    if (event.code === 'KeyP' && event.ctrlKey && event.shiftKey) {
        if(!document.pointerLockElement)
        {
            app.VideoElement.requestPointerLock();
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
    if (event.code === 'KeyF' && event.ctrlKey && event.shiftKey) {
        if (document.fullscreenElement === null) 
        {
            app.enterFullscreen();
            event.preventDefault();
            return;
        }
    }
    var Keyboard =
    {
        "Opcode":HidOpcode.KEYDOWN,
        "wVk":event.code,
    }

    app.HidDC.send(JSON.stringify(Keyboard));

    // disable problematic browser shortcuts
    if (event.code === 'F5' && event.ctrlKey ||
        event.code === 'Tab' ||
        event.code === 'KeyI' && event.ctrlKey && event.shiftKey ||
        event.code === 'KeyW' && event.ctrlKey ||
        event.code === 'F11') {
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
        (clientX - app.Mouse.mouseOffsetX - app.Mouse.centerOffsetX + app.Mouse.scrollX) * app.Mouse.mouseMultiX
    );

    if (serverX === app.Mouse.frameW - 1) serverX = app.Mouse.frameW;
    if (serverX > app.Mouse.frameW) serverX = app.Mouse.frameW;
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
        (clientY - app.Mouse.mouseOffsetY - app.Mouse.centerOffsetY + app.Mouse.scrollY)
            * app.Mouse.mouseMultiY);

    if (serverY === app.Mouse.frameH - 1) serverY = app.Mouse.frameH;
    if (serverY > app.Mouse.frameH) serverY = app.Mouse.frameH;
    if (serverY < 0) serverY = 0;

    return  Math.round(serverY);
}


/**
 * When fullscreen is entered, request keyboard and pointer lock.
 */
function 
onFullscreenChange() 
{
    if (document.fullscreenElement !== null) 
    {
        // Enter fullscreen
        //allow capture function key (ctrl, shift, tab)

        // app.VideoElement.requestPointerLock();
        // requestKeyboardLock();
    }
    reset_keyboard();
}



function
mouseLeaveEvent(event)
{
    reset_keyboard();
    reset_mouse();
}


/**
 * Attaches input event handles to docuemnt, window and element.
 */
function 
AttachEvent() 
{

    /**
     * determine screen parameter before connect HID event handler
     */
    windowCalculate();

    /**
     * full screen event
     */
    app.EventListeners.push(addListener(app.VideoElement.parentElement, 'fullscreenchange', onFullscreenChange, null));

    /**
     * video event
     */
    app.EventListeners.push(addListener(app.VideoElement, 'contextmenu', contextMenu, null)); ///disable content menu key on remote control

    /**
     * mouse event
     */
    app.EventListeners.push(addListener(app.VideoElement, 'wheel', mouseWheel, null));
    app.EventListeners.push(addListener(app.VideoElement, 'mousemove', mouseButtonMovement, null));
    app.EventListeners.push(addListener(app.VideoElement, 'mousedown', mouseButtonDown, null));
    app.EventListeners.push(addListener(app.VideoElement, 'mouseup', mouseButtonUp, null));
    app.EventListeners.push(addListener(app.VideoElement, 'mouseleave', mouseLeaveEvent, null));


    /**
     * mouse lock event
     */
    app.EventListeners.push(addListener(document, 'pointerlockchange', pointerLock, null));
    
    /**
     * keyboard event
     */
    app.EventListeners.push(addListener(window, 'keydown', keydown, null));
    app.EventListeners.push(addListener(window, 'keyup', keyup, null));

    /**
     * window resize event
     */
    app.EventListeners.push(addListener(app.VideoElement, 'resize', windowCalculate, null));
    app.EventListeners.push(addListener(window, 'resize', windowCalculate, null));

    /**
     * scroll event
     */
    app.EventListeners.push(addListener(window, 'scroll', () => 
    {
        app.Mouse.scrollX = window.scrollX;
        app.Mouse.scrollY = window.scrollY;
    }, app));

}


/**
 * Sends WebRTC app command to toggle display of the remote mouse pointer.
 */
function pointerLock() {
    if (document.pointerLockElement) {
        app.Mouse.relativeMouse = true;
        var INPUT =
        {
            "Opcode":HidOpcode.POINTER_LOCK,
            "Value":true
        }
        app.HidDC.send(JSON.stringify(INPUT));
    } else {        
        app.Mouse.relativeMouse = false;
        var INPUT =
        {
            "Opcode":HidOpcode.POINTER_LOCK,
            "Value":false
        }
        app.HidDC.send(JSON.stringify(INPUT));
    }
}


function 
DetachEvent() 
{
    removeListeners(app.EventListeners);
    document.exitPointerLock();
    reset_keyboard();
}

/**
 * Request keyboard lock, must be in fullscreen mode to work.
 */
function 
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
    Navigator.keyboard.lock(keys).then(
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

