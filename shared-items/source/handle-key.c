/**
 * @file shortcut.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2022-02-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <handle-key.h>

#include <glib-2.0/glib.h>
#include <json-glib/json-glib.h>

#include <enum.h>
#include <capture-key.h>


#include <Windows.h>
#include <gst/gst.h>

#ifdef G_OS_WIN32
/**
 * @brief 
 * true if mouse movement is relative
 */
static gboolean relative_mouse = FALSE;


typedef struct _HIDHandler
{
    /**
     * @brief 
     * 
     */
    gfloat screenwidth;

    /**
     * @brief 
     * 
     */
    gfloat screenheight;

    GstElement* capture;

    Shortcut shortcuts[20];

    gboolean active;
}HIDHandler;

static HIDHandler HID_handler = {0};

void
activate_hid_handler(GstElement* capture, 
                     Shortcut* shortcuts)
{
    HID_handler.capture = capture;

    GstCaps* cap = gst_element_get_static_pad(capture, "src");
    GstStructure* structure = gst_caps_get_structure(cap,0);
    gst_structure_get_int(structure,"width",&HID_handler.screenwidth);
    gst_structure_get_int(structure,"height",&HID_handler.screenheight);

    gint i = 0;
    Shortcut* temp = shortcuts;
    while (temp)
    {
        temp = shortcuts + i;
        temp->active = TRUE;
        memcpy(&(HID_handler.shortcuts[i]),temp,sizeof(Shortcut));
        i++;
    }
}


void
deactivate_hid_handler()
{
    memset(&HID_handler,0,sizeof(HIDHandler));
}

/**
 * @brief 
 * convert mouse input from json format to window format
 * @param input 
 * @param message 
 * @param core 
 */
static void
convert_mouse_input(INPUT* input, 
                    JsonObject* message)
{
    gfloat screenwidth = HID_handler.screenwidth;
    gfloat screenheight = HID_handler.screenheight; 

    input->mi.dx =  relative_mouse ? (LONG)
      ((gfloat)json_object_get_int_member(message, "dX")) :
    ((((gfloat)json_object_get_int_member(message, "dX"))/screenwidth)*65535);
    input->mi.dy =  relative_mouse ? (LONG)
      ((gfloat)json_object_get_int_member(message, "dY")) :
    ((((gfloat)json_object_get_int_member(message, "dY"))/screenheight)*65535);
}

/**
 * @brief 
 * get and set the visibility of pointer
 * @param capture 
 */
static void
toggle_pointer(GstElement* capture)
{
    gboolean toggle;
    g_object_get(capture, "show-cursor", &toggle, NULL); 
    toggle = !toggle;
    relative_mouse = !toggle;
    g_object_set(capture, "show-cursor", toggle, NULL); 
}


static gboolean
handle_shortcut(HIDHandler* handler,
                gint opcode)
{
    gint i = 0;
    while (handler->shortcuts[i].active)
    {
        Shortcut shortcut = handler->shortcuts[i];
        if(opcode == shortcut.opcode)
        {
            shortcut.function(shortcut.data);
            return TRUE;
        }
    }
    return FALSE;
}



/**
 * @brief 
 * 
 * @param message 
 * @param core 
 */
void
handle_input_javascript(gchar* message)
{
    JsonParser* parser = json_parser_new();
    JsonObject* object = get_json_object_from_string(message,NULL,parser);

	if(object == NULL) 
        return;

    INPUT window_input;
    gint button = 0;
    memset(&window_input,0, sizeof(window_input));
    JavaScriptOpcode opcode = json_object_get_int_member(object, "Opcode");

    if(handle_shortcut(&HID_handler,opcode))
        return;

    if(opcode == MOUSE_DOWN || opcode == MOUSE_UP || opcode == MOUSE_MOVE)
    {
        window_input.type = INPUT_MOUSE;
        window_input.mi.mouseData = 0;
        window_input.mi.time = 0;
        button = ( opcode != MOUSE_MOVE ) ? json_object_get_int_member(object, "button") : 0; 
        convert_mouse_input(&window_input,object);
    }

    else if(opcode == KEYUP || opcode == KEYDOWN)
    {
        window_input.type = INPUT_MOUSE;
        window_input.type = INPUT_KEYBOARD;
        window_input.ki.wVk = convert_javascript_key_to_window_key(
            json_object_get_string_member(object, "wVk"));
        window_input.ki.time = 0;
    }


    if (opcode == MOUSE_UP)
    {
        if(relative_mouse)
        {
            if(button == 0)
                window_input.mi.dwFlags =  MOUSEEVENTF_LEFTUP | MOUSEEVENTF_VIRTUALDESK;
            else if(button == 1)
                window_input.mi.dwFlags =  MOUSEEVENTF_MIDDLEUP | MOUSEEVENTF_VIRTUALDESK;
            else if (button == 2)
                window_input.mi.dwFlags =  MOUSEEVENTF_RIGHTUP | MOUSEEVENTF_VIRTUALDESK;
        }
        else
        {
            if(button == 0)
                window_input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP | MOUSEEVENTF_VIRTUALDESK;
            else if(button == 1)
                window_input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MIDDLEUP | MOUSEEVENTF_VIRTUALDESK;
            else if (button == 2)
                window_input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTUP | MOUSEEVENTF_VIRTUALDESK;
        }
    }
    else if (opcode == MOUSE_DOWN)
    {
        if(relative_mouse)
        {
            if(button == 0)
                window_input.mi.dwFlags =  MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_VIRTUALDESK;
            else if(button == 1)
                window_input.mi.dwFlags =  MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_VIRTUALDESK;
            else if (button == 2)
                window_input.mi.dwFlags =  MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_VIRTUALDESK;
        }
        else
        {
            if(button == 0)
                window_input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_VIRTUALDESK;
            else if(button == 1)
                window_input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_VIRTUALDESK;
            else if (button == 2) 
                window_input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_VIRTUALDESK;
        }

    }
    else if (opcode == MOUSE_MOVE)
    {
        if(relative_mouse)
            window_input.mi.dwFlags = MOUSEEVENTF_MOVE;
        else
            window_input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
    }
    else if(opcode == MOUSE_WHEEL)
    {
        window_input.mi.dwFlags = MOUSEEVENTF_WHEEL;
        window_input.mi.mouseData = (LONG)json_object_get_int_member(object, "WheeldY");
    }
    else if (opcode == KEYUP)
    {
        window_input.ki.dwFlags = KEYEVENTF_KEYUP;
    }
    else if (opcode == KEYDOWN)
    {
        //do nothing
    }

    SendInput(1, &window_input, sizeof(window_input));
    g_object_unref(parser);
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
handle_input_win32(gchar* message)
{
    JsonParser* parser = json_parser_new();
    JsonObject* object = get_json_object_from_string(message,NULL,parser);

	if(!object) 
        return;


    Win32Opcode opcode = json_object_get_int_member(object, "Opcode");

    if(handle_shortcut(&HID_handler,opcode))
        return;

    INPUT window_input;
    memset(&window_input,0, sizeof(INPUT));
    switch (opcode)
    {
        case KEYRAW:
            window_input.type =  INPUT_KEYBOARD;
            window_input.ki.wVk =     json_object_get_int_member(object, "wVk");
            window_input.ki.dwFlags = json_object_get_boolean_member(object, "IsUp") ? KEYEVENTF_KEYUP : 0;
            break;
        case MOUSERAW:
            window_input.type = INPUT_MOUSE;
            window_input.mi.dx = (LONG)(json_object_get_int_member(object, "dX"));
            window_input.mi.dy = (LONG)(json_object_get_int_member(object, "dY"));
            window_input.mi.dwFlags = convert_mouse_code(json_object_get_int_member(object, "MouseCode"));
            break;
        case MOUSEWHEEL:
            window_input.type = INPUT_MOUSE;
            window_input.mi.dwFlags = MOUSEEVENTF_WHEEL;
            window_input.mi.mouseData = (LONG)json_object_get_int_member(object, "WheeldY");
            break;
        case GAMEPAD_IN:
            /* code */
            break;
    }
    SendInput(1, &window_input, sizeof(window_input));
    g_object_unref(parser);
}

void            
handle_input_gtk(gchar* message)
{

}

#else

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