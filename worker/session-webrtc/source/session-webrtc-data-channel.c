/**
 * @file session-webrtc-data-channel.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <session-webrtc-data-channel.h>
#include <session-webrtc.h>
#include <session-webrtc-type.h>
#include <session-webrtc-pipeline.h>
#include <session-webrtc-remote-config.h>


#include <logging.h>
#include <human-interface-opcode.h>
#include <key-convert.h>
#include <development.h>

#include <gst/gst.h>
#include <glib-2.0/glib.h>
#include <gst/webrtc/webrtc_fwd.h>
#include <libsoup/soup.h>











struct _WebRTCHub
{
    /**
     * @brief 
     * hid datachannel for transfering human interface signal with client 
     */
    GObject* hid;

    /**
     * @brief 
     * control datachannel for transfering control signal with client
     */
    GObject* control;

    /**
     * @brief 
     * true if mouse movement is relative
     */
    gboolean relative_mouse;
};

WebRTCHub* 
webrtchub_initialize()
{
    WebRTCHub* hub = malloc(sizeof(WebRTCHub));
    hub->relative_mouse = FALSE;
    return hub;
}


void            handle_input_win32              (gchar* message,  
                                                SessionCore* core);

void            handle_input_gtk                 (gchar* message,  
                                                SessionCore* core);

void            handle_input_javascript         (gchar* message, 
                                                SessionCore* core);


#ifdef G_OS_WIN32
#include <Windows.h>

/**
 * @brief 
 * (IMPLEMENTATION SPECIFIC)
 * handle win32 key
 * @param message 
 * @param core 
 */
void
handle_input_win32(gchar* message,  
                    SessionCore* core);

/**
 * @brief 
 * convert mouse input from json format to window format
 * @param input 
 * @param message 
 * @param core 
 */
static void
convert_mouse_input(INPUT* input, 
                    JsonObject* message,
                    SessionCore* core)
{
    StreamConfig* qoe = session_core_get_qoe(core);
    WebRTCHub* hub = session_core_get_rtc_hub(core);

    static gfloat screenwidth, screenheight; 
    screenwidth =   qoe_get_screen_width(qoe);
    screenheight =  qoe_get_screen_height(qoe);

    if(hub->relative_mouse)
    {
        input->mi.dx = (LONG)
        ((gfloat)json_object_get_int_member(message, "dX"));
        input->mi.dy = (LONG)
        ((gfloat)json_object_get_int_member(message, "dY"));
    }
    else
    {
        input->mi.dx = (LONG)
        ((((gfloat)json_object_get_int_member(message, "dX"))/screenwidth)*65535);
        input->mi.dy = (LONG)
        ((((gfloat)json_object_get_int_member(message, "dY"))/screenheight)*65535);
    }
}

/**
 * @brief 
 * convert from mouse input code captured by win32 application to corresponding mouse input
 * @param input 
 * @return gint 
 */
static gint 
convert_mouse_code(gint input)
{
    switch (input)
    {
        case WM_MOUSEMOVE   :
            return MOUSEEVENTF_MOVE        ;
        case WM_LBUTTONDOWN :
            return MOUSEEVENTF_LEFTDOWN    ;
        case WM_LBUTTONUP   :
            return MOUSEEVENTF_LEFTUP      ;
        case WM_RBUTTONDOWN :
            return MOUSEEVENTF_RIGHTDOWN   ;
        case WM_RBUTTONUP   :
            return MOUSEEVENTF_RIGHTUP     ;
        case WM_MBUTTONDOWN :
            return MOUSEEVENTF_MIDDLEDOWN  ;
        case WM_MBUTTONUP   :
            return MOUSEEVENTF_MIDDLEUP    ;
    }
}
void
handle_input_javascript(gchar* message, 
                        SessionCore* core)
{
    GError* error = NULL;
    JsonParser* parser = json_parser_new();
    JsonObject* object = get_json_object_from_string(message,&error,parser);
	if(!error == NULL || object == NULL) {return;}

    WebRTCHub* hub = session_core_get_rtc_hub(core); 
    JavaScriptOpcode opcode = json_object_get_int_member(object, "Opcode");

    if (opcode == MOUSE_UP)
    {
        INPUT mouse;
        memset(&mouse,0, sizeof(mouse));
        mouse.type = INPUT_MOUSE;
        gint button = json_object_get_int_member(object, "button");
        if(hub->relative_mouse)
        {
            if(button == 0){
                mouse.mi.dwFlags =  MOUSEEVENTF_LEFTUP | MOUSEEVENTF_VIRTUALDESK;
            }else if(button == 1){
                mouse.mi.dwFlags =  MOUSEEVENTF_MIDDLEUP | MOUSEEVENTF_VIRTUALDESK;
            }else if (button == 2) {
                mouse.mi.dwFlags =  MOUSEEVENTF_RIGHTUP | MOUSEEVENTF_VIRTUALDESK;
            }
        }
        else
        {
            convert_mouse_input(&mouse,object,core);
            if(button == 0){
                mouse.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP | MOUSEEVENTF_VIRTUALDESK;
            }else if(button == 1){
                mouse.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MIDDLEUP | MOUSEEVENTF_VIRTUALDESK;
            }else if (button == 2) {
                mouse.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTUP | MOUSEEVENTF_VIRTUALDESK;
            }
        }
        mouse.mi.mouseData = 0;
        mouse.mi.time = 0;
        SendInput(1, &mouse, sizeof(mouse));
    }
    else if (opcode == MOUSE_DOWN)
    {
        INPUT mouse;
        memset(&mouse,0, sizeof(mouse));
        mouse.type = INPUT_MOUSE;
        gint button = json_object_get_int_member(object, "button");
        if(hub->relative_mouse)
        {
            if(button == 0){
                mouse.mi.dwFlags =  MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_VIRTUALDESK;
            }else if(button == 1){
                mouse.mi.dwFlags =  MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_VIRTUALDESK;
            }else if (button == 2) {
                mouse.mi.dwFlags =  MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_VIRTUALDESK;
            }
        }
        else
        {
            convert_mouse_input(&mouse,object,core);
            if(button == 0){
                mouse.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_VIRTUALDESK;
            }else if(button == 1){
                mouse.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_VIRTUALDESK;
            }else if (button == 2) {
                mouse.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_VIRTUALDESK;
            }
        }
        mouse.mi.mouseData = 0;
        mouse.mi.time = 0;
        SendInput(1, &mouse, sizeof(mouse));
    }
    else if (opcode == MOUSE_MOVE)
    {
        INPUT mouse;
        memset(&mouse,0, sizeof(mouse));
        mouse.type = INPUT_MOUSE;
        convert_mouse_input(&mouse,object,core);
        if(hub->relative_mouse)
        {
            mouse.mi.dwFlags = MOUSEEVENTF_MOVE;
        }
        else
        {
            mouse.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
        }
        SendInput(1, &mouse, sizeof(mouse));
    }
    else if(opcode == MOUSE_WHEEL)
    {
        INPUT mouse;
        memset(&mouse,0, sizeof(mouse));
        mouse.type = INPUT_MOUSE;
        convert_mouse_input(&mouse,object,core);
        mouse.mi.dwFlags = MOUSEEVENTF_WHEEL;
        mouse.mi.mouseData = (LONG)json_object_get_int_member(object, "WheeldY");
        mouse.mi.time = 0;
        SendInput(1, &mouse, sizeof(mouse));
    }
    else if (opcode == KEYUP)
    {
        INPUT keyboard;
        memset(&keyboard,0, sizeof(keyboard));
        keyboard.type = INPUT_KEYBOARD;
        keyboard.ki.wVk = convert_javascript_key_to_window_key(
            json_object_get_string_member(object, "wVk"));
        keyboard.ki.dwFlags = KEYEVENTF_KEYUP;
        keyboard.ki.time = 0;
        SendInput(1, &keyboard, sizeof(keyboard));
    }
    else if (opcode == KEYDOWN)
    {
        INPUT keyboard;
        memset(&keyboard,0, sizeof(keyboard));
        keyboard.type =  INPUT_KEYBOARD;
        keyboard.ki.wVk = convert_javascript_key_to_window_key(
            json_object_get_string_member(object, "wVk"));
        keyboard.ki.time = 0;
        SendInput(1, &keyboard, sizeof(keyboard));
    }
    else if (opcode == POINTER_LOCK)
    {
        gboolean relative = json_object_get_boolean_member(object,"Value");
        hub->relative_mouse = relative; 
        toggle_pointer(relative,core);
    }
    else if(opcode == QOE_REPORT)
    {
        JsonObject* object = json_object_get_string_member(object,"Value");
        if(!object){return;}
        const gchar* http_aliases[] = { "http", NULL };

        GString* string = g_string_new("http://");
        g_string_append(string,"220");
        g_string_append(string,":2220/session/SessionMetric");
        gchar* uri = g_string_free(string,FALSE);


        SoupSession* session = soup_session_new_with_options(
                SOUP_SESSION_SSL_STRICT, FALSE,
                SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
                SOUP_SESSION_HTTPS_ALIASES, http_aliases, NULL);

        SoupMessage* message = soup_message_new(SOUP_METHOD_POST,uri);

        gchar* msgjson = get_string_from_json_object(object);
        soup_message_set_request(message,"application/json",SOUP_MEMORY_COPY
            ,msgjson,strlen(msgjson));
        soup_session_send_async(session,message,NULL,NULL,NULL);
    }
    g_object_unref(parser);
}

void
handle_input_win32(gchar* message,  
                    SessionCore* core)
{
    GError* error = NULL;
    JsonParser* parser = json_parser_new();
    JsonObject* object = get_json_object_from_string(message,&error,parser);
	if(!error == NULL || object == NULL) {return;}

    WebRTCHub* hub = session_core_get_rtc_hub(core); 
    Win32Opcode opcode = json_object_get_int_member(object, "Opcode");

    switch (opcode)
    {
        case KEYRAW:
            INPUT keyboard;
            gboolean isup = json_object_get_boolean_member(object, "IsUp");
            memset(&keyboard,0, sizeof(INPUT));
            keyboard.type =  INPUT_KEYBOARD;
            keyboard.ki.wVk = json_object_get_int_member(object, "wVk");
            keyboard.ki.time = 0;
            keyboard.ki.dwFlags = isup ? KEYEVENTF_KEYUP : 0;
            SendInput(1, &keyboard, sizeof(keyboard));
            break;
        case MOUSERAW:
            INPUT mouse;
            memset(&mouse,0, sizeof(INPUT));
            mouse.type = INPUT_MOUSE;
            mouse.mi.dx = (LONG)(json_object_get_int_member(object, "dX"));
            mouse.mi.dy = (LONG)(json_object_get_int_member(object, "dY"));
            mouse.mi.dwFlags = convert_mouse_code(json_object_get_int_member(object, "MouseCode"));
            mouse.mi.mouseData = 0;
            mouse.mi.time = 0;
            SendInput(1, &mouse, sizeof(mouse));
            break;
        case MOUSEWHEEL:
            INPUT wheel;
            memset(&wheel,0, sizeof(INPUT));
            wheel.type = INPUT_MOUSE;
            wheel.mi.dwFlags = MOUSEEVENTF_WHEEL;
            wheel.mi.mouseData = (LONG)json_object_get_int_member(object, "WheeldY");
            wheel.mi.time = 0;
            SendInput(1, &wheel, sizeof(wheel));
            break;
        case GAMEPAD_IN:
            /* code */
            break;
    }

    g_object_unref(parser);
}
void            
handle_input_gtk(gchar* message,  
                SessionCore* core)
{

}
#else

#include <Xlib.h>

void            
handle_input_win32(gchar* message,  
                   SessionCore* core)
{
    GError* error = NULL;
    JsonParser* parser = json_parser_new();
    JsonObject* object = get_json_object_from_string(message,&error,parser);
	if(!error == NULL || object == NULL) {return;}

    WebRTCHub* hub = session_core_get_rtc_hub(core); 
    Win32Opcode opcode = json_object_get_int_member(object, "Opcode");

}


void            
handle_input_javascript(gchar* message, 
                        SessionCore* core)
{
    GError* error = NULL;
    JsonParser* parser = json_parser_new();
    JsonObject* object = get_json_object_from_string(message,&error,parser);
	if(!error == NULL || object == NULL) {return;}

    WebRTCHub* hub = session_core_get_rtc_hub(core); 
    JavaScriptOpcode opcode = json_object_get_int_member(object, "Opcode");
}
#include <keysym.h>




XKeyEvent 
createKeyEvent(Display *display, 
                Window win,
                Window winRoot, 
                gboolean press,
                int keycode, 
                int modifiers)
{
	XKeyEvent event;

	event.display     = display;
	event.window      = win;
	event.root        = winRoot;
	event.subwindow   = None;
	event.time        = CurrentTime;
	event.x           = 1;
	event.y           = 1;
	event.x_root      = 1;
	event.y_root      = 1;
	event.same_screen = True;
	event.keycode     = XKeysymToKeycode(display, keycode);
	event.state       = modifiers;

	if(press)
		event.type = KeyPress;
	else
		event.type = KeyRelease;

	return event;
}


void
stimulate_mouse_event(SessionCore* core)
{
    Display* display = session_core_display_interface(core);

    // Get the root window for the current display.
	Window winRoot = XDefaultRootWindow(display);

    // Find the window which has the current keyboard focus.
	Window winFocus;
	int    revert;
	XGetInputFocus(display, &winFocus, &revert);
    

    XKeyEvent event = createKeyEvent(display, winFocus, winRoot, TRUE, XK_Down, 0);
	XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);

// Send a fake key release event to the window.
	event = createKeyEvent(display, winFocus, winRoot, FALSE, XK_Down, 0);
	XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);
}

#endif






/**
 * @brief 
 * ignore binary message
 * @param datachannel 
 * @param byte 
 * @param core 
 */
static void
control_channel_on_message_data(GObject* datachannel,
    GBytes* byte,
    SessionCore* core)
{
    return;
}

/**
 * @brief 
 * ignore binary message
 * @param datachannel 
 * @param data 
 * @param core 
 */
static void
hid_channel_on_message_data(GObject* datachannel,
    GBytes* data,
    SessionCore* core)
{
    return;
}


/**
 * @brief 
 * handle message from hid datachannel and send to window
 * @param dc 
 * @param message 
 * @param core 
 */
static void
hid_channel_on_message_string(GObject* dc,
                            gchar* message,
                            SessionCore* core)
{
    gchar* package = malloc(1000);
    memset(package,0,1000);
    memcpy(package,message,strlen(message));


    CoreEngine engine = session_core_get_client_engine(core);
    DeviceType device = session_core_get_client_device(core);

    if(DEVELOPMENT_ENVIRONMENT)
    {
        g_print("%s\n",message);
    }


    if(engine == CHROME)
    {
        handle_input_javascript(package,core);
    }
    else if (engine == GSTREAMER)
    {
        if(device == WINDOW_APP)
        {
            handle_input_win32(package,core);
        }
        else if(device == LINUX_APP)
        {
            handle_input_gtk(package,core);
        }
    }
    free(package);
}






/**
 * @brief 
 * handle control data channel
 * @param dc 
 * @param message 
 * @param core 
 */
static void
control_channel_on_message_string(GObject* dc,
                                  gchar* message,
                                  SessionCore* core)
{
    WebRTCHub* hub = session_core_get_rtc_hub(core);
    g_signal_emit_by_name(dc,"send-string",message,NULL);
}


/**
 * @brief 
 * handle datachannel open signal
 * @param dc 
 * @param core 
 */
static void
channel_on_open(GObject* dc,
                SessionCore* core)
{
    return;
}


/**
 * @brief 
 * handle datachannel close and error signal
 * @param dc 
 * @param core 
 */
static void
channel_on_close_and_error(GObject* dc,
    SessionCore* core)
{
    return;
}








gboolean
connect_data_channel_signals(SessionCore* core)
{
    WebRTCHub* hub = session_core_get_rtc_hub(core);
    Pipeline* pipe = session_core_get_pipeline(core);
    GstElement* webrtcbin = pipeline_get_webrtc_bin(pipe);


    // connect data channel source
    g_signal_emit_by_name(webrtcbin, "create-data-channel", "Control", 
        NULL, &hub->control);
    g_signal_emit_by_name(webrtcbin, "create-data-channel", "HID", 
        NULL, &hub->hid);
    
    g_signal_connect(hub->hid, "on-error",
        G_CALLBACK(channel_on_close_and_error), core);
    g_signal_connect(hub->hid, "on-open",
        G_CALLBACK(channel_on_open), core);
    g_signal_connect(hub->hid, "on-close",
        G_CALLBACK(channel_on_close_and_error), core);
    g_signal_connect(hub->hid, "on-message-string",
        G_CALLBACK(hid_channel_on_message_string), core);
    g_signal_connect(hub->hid, "on-message-data",
        G_CALLBACK(hid_channel_on_message_data), core);

    g_signal_connect(hub->control, "on-error",
        G_CALLBACK(channel_on_close_and_error), core);
    g_signal_connect(hub->control, "on-open",
        G_CALLBACK(channel_on_open), core);
    g_signal_connect(hub->control, "on-close",
        G_CALLBACK(channel_on_close_and_error), core);
    g_signal_connect(hub->control, "on-message-string",
        G_CALLBACK(control_channel_on_message_string), core);
    g_signal_connect(hub->control, "on-message-data",
        G_CALLBACK(control_channel_on_message_data), core);
    return TRUE;
}



GstWebRTCDataChannel*
webrtc_hub_get_control_data_channel(WebRTCHub* hub)
{
    return hub->control;
}