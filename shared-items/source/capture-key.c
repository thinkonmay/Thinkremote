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

#include <enum.h>
#include <overlay-gui.h>

#include <glib-2.0/glib.h>
#include <json-glib/json-glib.h>
#include <json-handler.h>


typedef struct _HidInput
{
    gdouble delta_x;
    gdouble delta_y;
    gint wheel_dY;

    gint mouse_code;
    gint keyboard_code;

    Win32Opcode opcode;
    gboolean key_is_up;
}HidInput;


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

    /**
     * @brief 
     * 
     */
    Shortcut shortcuts [20];

    /**
     * @brief 
     * 
     */
    POINT previous_cursor_position;

    /**
     * @brief 
     * 
     */
    gboolean enable;
};

static InputHandler HID_handler = {0};


/**
 * @brief 
 * 
 * @param input 
 */
static void             parse_hid_event             (HidInput* input);

/**
 * @brief 
 * 
 * @return gboolean 
 */
gboolean                handle_user_shortcut        ();


InputHandler*
init_input_capture_system(HIDHandleFunction function, 
                          Shortcut* shortcuts,
                          gpointer data)
{
    memset(&HID_handler,0,sizeof(InputHandler));
    HID_handler.handler = function;
    HID_handler.data = data;
    HID_handler.enable = FALSE;

    gint i = 0;
    while ((shortcuts + i)->active)
    {
        memcpy(&HID_handler.shortcuts[i],(shortcuts + i),sizeof(Shortcut));
        i++;
    }

    return &HID_handler;
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

static void
send_normal_event(HidInput* input)
{
    JsonObject* object = json_object_new();
    json_object_set_int_member(object,"Opcode",(gint)input->opcode);

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
    if(handle_user_shortcut())
        return;

    navigation->opcode = KEYRAW;
    navigation->key_is_up = input.Flags;
    navigation->keyboard_code = input.VKey;
}

static void
handle_window_mouse_move(RAWMOUSE input,
                        HidInput* navigation,
                        HWND window)
{
    if(!is_hover_window())
        return;

    navigation->opcode = MOUSERAW;
    navigation->mouse_code = WM_MOUSEMOVE;
    navigation->delta_x = input.lLastX;
    navigation->delta_y = input.lLastY;
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
    
    if (raw_input->header.dwType == RIM_TYPEMOUSE && 
        (raw_input->data.mouse.lLastX || raw_input->data.mouse.lLastY))
        handle_window_mouse_move(raw_input->data.mouse,&navigation,hwnd);

    if (navigation.opcode > 100)
        parse_hid_event(&navigation);
}



/**
 * @brief 
 * center mouse to center of the screee
 * @param gui 
 * @return RECT new position of the screen 
 */
POINT
center_mouse_position(HWND window)
{
    POINT pt;
    RECT rect;
    GetWindowRect(window,&rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    pt.x = width /2;
    pt.y = height /2;
    ClientToScreen(window, &pt);
    SetCursorPos(pt.x,pt.y);
    ScreenToClient(window,&pt);
    return pt; 
}

void
handle_mouse_button(gint mouse_code)
{
    if(mouse_code == MOUSE_MOVE)
        return;

    HidInput navigation = {0};
    navigation.opcode = MOUSERAW;
    navigation.mouse_code = mouse_code;
    navigation.delta_x = 0;
    navigation.delta_y = 0;

    parse_hid_event(&navigation);
}


void
handle_window_wheel(gint isup)
{
    HidInput navigation = {0};
    navigation.opcode    = MOUSEWHEEL;
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









/**
 * @brief 
 * parse human interface event 
 * @param input Hid structure to parse
 * @param core remote app
 */
static void
parse_hid_event(HidInput* input)
{
    if(!HID_handler.enable)
        return;

    switch((gint)input->opcode)
    {
        case MOUSERAW:
            send_mouse_signal(input);
            break;
        case MOUSEWHEEL:
            send_mouse_wheel_signal(input);
            break;
        case KEYRAW:
            send_key_event(input);
            break;
        default:
            send_normal_event(input);
            break;
    }
}


/**
 * @brief 
 * 
 * @return gboolean TRUE if the input signal should be disabled
 */
gboolean
handle_user_shortcut()
{
    gint i = 0;
    while(HID_handler.shortcuts[i].active)
    {
        Shortcut shortcut = HID_handler.shortcuts[i];

        gint k = 0;
        while (shortcut.key_list[k])
        {
            gint key = shortcut.key_list[k];
            if(!_keydown(key))
            {
                goto ignore;
            }
            k++;
        }
        HidInput input = {0};
        input.opcode = shortcut.opcode;
        parse_hid_event(&input);

        if(shortcut.function && shortcut.data)
            shortcut.function(shortcut.data);
        else if (shortcut.function)
            shortcut.function(NULL);

        return TRUE;
ignore:
        i++;
    }
    return FALSE;
}



void
trigger_hotkey_by_opcode(ShortcutOpcode opcode)
{
    gint i = 0;
    while(HID_handler.shortcuts[i].active)
    {
        Shortcut shortcut = HID_handler.shortcuts[i];
        if(shortcut.opcode == opcode)
        {
            HidInput input = {0};
            input.opcode = opcode;
            parse_hid_event(&input);

            if(shortcut.function && shortcut.data)
                shortcut.function(shortcut.data);
            else if (shortcut.function)
                shortcut.function(NULL);
        }
        i++;
    }
}

void                
toggle_input_capture(InputHandler* handler)
{
    /**
     * @brief 
     * reset mouse and keyboard to prevent key stuck
     */
    trigger_hotkey_by_opcode(RESET_KEY);
    handler->enable = !handler->enable;
}




#endif