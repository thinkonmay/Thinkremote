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
#include <shortcut.h>
#include <handle-key.h>

#include <glib-2.0/glib.h>
#include <json-glib/json-glib.h>

#include <enum.h>
#include <capture-key.h>
#include <json-handler.h>
#include <logging.h>
#include <environment.h>
#include <global-var.h>


#include <Windows.h>
#include <gst/gst.h>

#ifdef G_OS_WIN32


struct _HIDHandler
{
    /**
     * @brief 
     * 
     */
    gint screenwidth;

    /**
     * @brief 
     * 
     */
    gint screenheight;

    GstElement* capture;

    Shortcut shortcuts[20];

    gboolean active;

    /**
     * @brief 
     * true if mouse movement is relative
     */
    gboolean relative_mouse;
};

static HIDHandler HID_handler = {0};


/**
 * @brief 
 * 
 * @param data 
 */
void            reset_session_key               (gpointer data);



/**
 * @brief Set the relative mouse object
 * 
 * @param isTrue 
 */
void            set_relative_mouse              (gboolean isTrue);

HIDHandler*
activate_hid_handler(GstElement* capture, 
                     Shortcut* shortcuts)
{
    HID_handler.capture = capture;
    HID_handler.relative_mouse = TRUE;
    
    add_new_shortcut_to_list(shortcuts,NULL,RESET_KEY,reset_session_key,NULL);
    add_new_shortcut_to_list(shortcuts,NULL,RELATIVE_MOUSE_ON,set_relative_mouse,GINT_TO_POINTER(TRUE));
    add_new_shortcut_to_list(shortcuts,NULL,RELATIVE_MOUSE_OFF,set_relative_mouse,GINT_TO_POINTER(FALSE));

    GstPad* pad = gst_element_get_static_pad(capture, "src");
    GstCaps* caps = gst_pad_get_current_caps (pad);
    if (!caps)
        caps = gst_pad_query_caps (pad, NULL);

    GstStructure* structure = gst_caps_get_structure(caps,0);
    gchar* output = gst_structure_serialize(structure,GST_SERIALIZE_FLAG_NONE);
    gst_structure_get_int(structure,"width",&HID_handler.screenwidth);
    gst_structure_get_int(structure,"height",&HID_handler.screenheight);

    gint i = 0;
    while ((shortcuts + i)->active)
    {
        memcpy(&(HID_handler.shortcuts[i]),(shortcuts + i),sizeof(Shortcut));
        i++;
    }



    return &HID_handler;
}


void
deactivate_hid_handler(HIDHandler* handle)
{
    memset(handle,0,sizeof(HIDHandler));
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

    input->mi.dx =  HID_handler.relative_mouse ? (LONG)
      ((gfloat)json_object_get_int_member(message, "dX")) :
    ((((gfloat)json_object_get_int_member(message, "dX"))/screenwidth)*65535);
    input->mi.dy =  HID_handler.relative_mouse ? (LONG)
      ((gfloat)json_object_get_int_member(message, "dY")) :
    ((((gfloat)json_object_get_int_member(message, "dY"))/screenheight)*65535);
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
            if(shortcut.function && shortcut.data)
                shortcut.function(shortcut.data);
            else if (shortcut.function)
                shortcut.function(NULL);

            gchar buffer[10] = {0};
            itoa(opcode,buffer,10);
            GString* string = g_string_new("Handled client event with opcode: ");
            g_string_append(string,buffer);
            worker_log_output(g_string_free(string,FALSE));
            return TRUE;
        }
        i++;
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
        window_input.type = INPUT_KEYBOARD;
        window_input.ki.wVk = convert_javascript_key_to_window_key(
            json_object_get_string_member(object, "wVk"));
        window_input.ki.time = 0;
    }


    if (opcode == MOUSE_UP)
    {
        if(HID_handler.relative_mouse)
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
        if(HID_handler.relative_mouse)
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
        if(HID_handler.relative_mouse)
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

    if(!DEVELOPMENT_ENVIRONMENT)
        SendInput(1, &window_input, sizeof(window_input));
    g_object_unref(parser);
}


void
set_relative_mouse(gboolean isTrue)
{
    HID_handler.relative_mouse = isTrue;
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
feedback_mouse_position()
{
    // GetCursorPos()

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

    gint mouse_code;
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
            mouse_code = json_object_get_int_member(object, "MouseCode");
            window_input.mi.dwFlags = convert_mouse_code(mouse_code);
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

    if(!DEVELOPMENT_ENVIRONMENT)
        SendInput(1, &window_input, sizeof(window_input));
    g_object_unref(parser);

    // if(opcode == MOUSERAW && mouse_code == WM_MOUSEMOVE)
}

void            
handle_input_gtk(gchar* message)
{

}

static gint reset_key_array[10] = 
{
    VK_SHIFT,
    VK_CONTROL,
    VK_LWIN,
    VK_RWIN,
    VK_ESCAPE,
    VK_MENU,
    0,
};
static gint reset_mouse_array[10] = 
{
    WM_LBUTTONUP,
    WM_RBUTTONUP,
    WM_MBUTTONUP,
    0
};

static gint reset_mouse_virtual_code[10] = 
{
    VK_LBUTTON,
    VK_RBUTTON,
    VK_MBUTTON,
    0
};

/**
 * @brief 
 * detect if a key is pressed
 * @param key 
 * @return gboolean 
 */
static gboolean
_keydown(int *key)
{
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

void
reset_session_key(gpointer data)
{
    if(DEVELOPMENT_ENVIRONMENT)
        return;


    gint i = 0;
    while (!reset_mouse_array[i])
    {
        if(_keydown(reset_mouse_virtual_code[i]))
        {
            INPUT window_input;
            memset(&window_input,0,sizeof(INPUT));
            window_input.type = INPUT_MOUSE;
            window_input.mi.dwFlags = reset_mouse_array[i];
            SendInput(1, &window_input, sizeof(window_input));
        }
        i++;
    }

    i = 0;
    while (!reset_key_array[i])
    {
        if(_keydown(reset_key_array[i]))
        {
            INPUT window_input;
            memset(&window_input,0,sizeof(INPUT));
            window_input.ki.wVk =     reset_key_array[i];
            window_input.type =       INPUT_KEYBOARD;
            window_input.ki.dwFlags = KEYEVENTF_KEYUP ;
            SendInput(1, &window_input, sizeof(window_input));
        }
        i++;
    }
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


