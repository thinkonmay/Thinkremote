/**
 * @file capture_key.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2022-02-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <capture-key.h>

#include <human-interface-opcode.h>

#include <glib-2.0/glib.h>
#include <json-glib/json-glib.h>


struct _HidInput
{
    gdouble delta_x;
    gdouble delta_y;
    gint wheel_dY;

    gint mouse_code;
    gint keyboard_code;


    Win32Opcode opcode;

    gboolean relative;
    gboolean key_is_up;
};


struct _InputHandler
{
    /**
     * @brief 
     * handle gamepad event
     */
    GThread *gamepad_thread;

    /**
     * @brief 
     * 
     */
    HIDHandleFunction handler;

    /**
     * @brief 
     * 
     */
    gpointer data;
};

static InputHandler HID_handler = {0};


InputHandler*
init_input_capture_system(HIDHandleFunction function, 
                          gpointer data)
{
    memset(&HID_handler,0,sizeof(InputHandler));
    HID_handler.handler = function;
    HID_handler.data = data;
    return &HID_handler;
}

void                
set_hid_handle_function(HIDHandleFunction function)
{
    HID_handler.handler = function;
}


static void
send_mouse_signal(HidInput* input)
{
    JsonObject* object = json_object_new();
    json_object_set_int_member(object,"Opcode",(gint)input->opcode);
    json_object_set_int_member(object,"MouseCode",(gint)input->mouse_code);
    json_object_set_int_member(object,"dX",(gint)input->delta_x);
    json_object_set_int_member(object,"dY",(gint)input->delta_y);

    if(HID_handler.handler)
        HID_handler.handler(get_string_from_json_object(object),HID_handler.data);
}

static void
send_mouse_wheel_signal(HidInput* input)
{
    JsonObject* object = json_object_new();
    json_object_set_int_member(object,"Opcode",(gint)input->opcode);
    json_object_set_int_member(object,"WheeldY",(gint)input->wheel_dY);

    if(HID_handler.handler)
        HID_handler.handler(get_string_from_json_object(object),HID_handler.data);
}

static void
send_key_event(HidInput* input)
{
    JsonObject* object = json_object_new();
    json_object_set_int_member(object,"Opcode",(gint)input->opcode);
    json_object_set_int_member(object,"wVk",input->keyboard_code);
    json_object_set_boolean_member(object,"IsUp",input->key_is_up);

    if(HID_handler.handler)
        HID_handler.handler(get_string_from_json_object(object),HID_handler.data);
}


#ifdef G_OS_WIN32

#include <Windows.h>
#pragma comment(lib, "XInput.lib")
#include <Xinput.h>
#include <Windows.h>

static void
send_gamepad_signal(HARDWAREINPUT input)
{
    JsonObject* object = json_object_new();
    json_object_set_int_member(object,"Opcode",GAMEPAD_IN);
    json_object_set_int_member(object,"uMsg",(gint)input.uMsg);
    json_object_set_int_member(object,"wParamH",(gint)input.wParamH);
    json_object_set_int_member(object,"wParamL",(gint)input.wParamL);

    if(HID_handler.handler)
        HID_handler.handler(get_string_from_json_object(object),HID_handler.data);
}

static void parse_hid_event(HidInput* input);
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

void
reset_mouse()
{
    gint i = 0;
    while (!reset_mouse_array[i])
    {
        if(_keydown(reset_mouse_virtual_code[i]))
        {
            HidInput input = {0};
            input.opcode = MOUSERAW;
            input.mouse_code = reset_mouse_array[i];
            send_mouse_signal(&input);
        }
        i++;
    }
}





/**
 * @brief 
 * 
 * https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawkeyboard
 * https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawkeyboard
 * @param input 
 * @param navigation 
 */
static void
handle_window_keyboard(RAWKEYBOARD input,
                        HidInput* navigation)
{
    gint Message= input.Message;
    navigation->opcode = KEYRAW;
    navigation->key_is_up = input.Flags;
    navigation->keyboard_code = input.VKey;
}



void 
handle_message_window_proc(HWND hwnd, 
                            UINT message, 
                            WPARAM wParam, 
                            LPARAM lParam)
{
    HidInput navigation = {0};

    guint dwSize;
    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));

    gpointer buffer = malloc(dwSize);
    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) 
        return;

    RAWINPUT* raw_input = (RAWINPUT*) buffer;
    if (raw_input->header.dwType == RIM_TYPEKEYBOARD) 
        handle_window_keyboard(raw_input->data.keyboard,&navigation); 
    
    parse_hid_event(&navigation);
}


void
handle_window_mouse_relative(gint mouse_code,
                          gint delta_X,
                          gint delta_Y)
{
    // reduce mouse move signal by filter unactive mouse
    if(!delta_X && !delta_Y && mouse_code == WM_MOUSEMOVE)
        return;

    HidInput navigation = {0};
    navigation.opcode = MOUSERAW;
    navigation.relative = TRUE;
    navigation.mouse_code = mouse_code;
    navigation.delta_x = delta_X;
    navigation.delta_y = delta_Y;

    parse_hid_event(&navigation);
}


void
handle_window_wheel(gint isup)
{
    HidInput navigation = {0};
    navigation.opcode    = MOUSE_WHEEL;
    navigation.wheel_dY  = isup ? 120 : -120;

    parse_hid_event(&navigation);
}


/**
 * @brief 
 * 
 * @param data 
 * @return gpointer 
 */
static gpointer 
gamepad_thread_func(gpointer data)
{
    XINPUT_STATE state     = {0};
    XINPUT_STATE prevstate = {0};

    DWORD dwResult = XInputGetState(0, &prevstate);

    if (dwResult != ERROR_SUCCESS)
        return;

    // Controller is connected
    while (XInputGetState(0, &state) == ERROR_SUCCESS)
    {
        //dwpacketnumber diff?
        if (state.dwPacketNumber != prevstate.dwPacketNumber)
        {
            // handle event
        }
        memcpy(&prevstate, &state, sizeof(XINPUT_STATE));
        Sleep(10);
    }
}

void
send_gamepad_vibration(XINPUT_VIBRATION vibration)
{
    XInputSetState(0, &vibration);
}

void
reset_keyboard()
{
    gint i = 0;
    while (!reset_key_array[i])
    {
        
        if(_keydown(reset_key_array[i]))
        {
            HidInput input = {0};
            input.opcode = KEYRAW;
            input.key_is_up = TRUE;
            input.keyboard_code = reset_key_array[i]; 
            send_key_event(&input);
        }
        i++;
    }
}








/**
 * @brief 
 * parse human interface event 
 * @param input Hid structure to parse
 * @param core remote app
 */
static void
parse_hid_event(HidInput* input)
{
    switch((gint)input->opcode)
    {
        case MOUSERAW:
            send_mouse_signal(input);
            break;
        case MOUSE_WHEEL:
            send_mouse_wheel_signal(input);
            break;
        case KEYRAW:
            send_key_event(input);
            break;
        default:
            return;
    }
}

#else
#include <gst/video/navigation.h>


gboolean      
handle_navigator(GstEvent *event, 
                RemoteApp* core)
{
    HidInput* navigation = malloc(sizeof(HidInput));
    gint eventcode = gst_navigation_event_get_type(event);\
    
    switch (eventcode)
    {
        case GST_NAVIGATION_EVENT_KEY_PRESS: 
            gst_navigation_event_parse_key_event(event,&(navigation->keyboard_code));
            navigation->opcode = KEYDOWN;
            break; 
        case GST_NAVIGATION_EVENT_KEY_RELEASE: 
            gst_navigation_event_parse_key_event(event,&(navigation->keyboard_code));
            navigation->opcode = KEYUP;
            break;
        case GST_NAVIGATION_EVENT_MOUSE_MOVE: 
            gst_navigation_event_parse_mouse_move_event(event,&(navigation->delta_x),&(navigation->delta_y));
            navigation->opcode = MOUSE_MOVE;
            break; 
        case GST_NAVIGATION_EVENT_MOUSE_BUTTON_PRESS: 
            gst_navigation_event_parse_mouse_button_event(event,&(navigation->mouse_code),&(navigation->delta_y),&(navigation->delta_y));
            navigation->opcode = MOUSE_DOWN;
            break; 
        case GST_NAVIGATION_EVENT_MOUSE_BUTTON_RELEASE: 
            gst_navigation_event_parse_mouse_button_event(event,&(navigation->mouse_code),&(navigation->delta_x),&(navigation->delta_y));
            navigation->opcode = MOUSE_UP;
            break; 
        default:
            break;
    }
    parse_hid_event(navigation);
    free(navigation);
}

#endif