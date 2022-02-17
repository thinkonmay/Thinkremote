/**
 * @file remote-udp-input.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <remote-udp-input.h>
#include <remote-udp-type.h>
#include <remote-udp-gui.h>


#include <glib.h>
#include <gst/gst.h>
#include <global-var.h>
#include <libsoup/soup.h>

#include <human-interface-opcode.h>
#include <message-form.h>
#include <development.h>


#ifdef G_OS_WIN32
#pragma comment(lib, "XInput.lib")
#include <Xinput.h>
#include <Windows.h>
#define WIN32_HID_CAPTURE
#endif

typedef struct _InputEndpoint
{
    /**
     * @brief 
     * 
     */
    gchar human_interface_port[50];
    
    /**
     * @brief 
     * 
     */
    gchar human_interface_ip[50];
}InputEndpoint;


struct _InputHandler
{
    /**
     * @brief 
     * reference to remote app
     */
    RemoteUdp* app;

    /**
     * @brief 
     * handle gamepad event
     */
    GThread *gamepad_thread;

    /**
     * @brief 
     * 
     */
    InputEndpoint endpoint;


    /**
     * @brief 
     * 
     */
    gboolean capturing;
};

static InputHandler HID_handler = {0}; 



InputHandler*
init_input_capture_system(RemoteUdp* app)
{
    InputHandler* handler = malloc(sizeof(InputHandler));
    memset(handler,0,sizeof(InputHandler));
    return handler;
}


void
setup_input_endpoint(InputHandler* handler,
                     gchar* input_ip,
                     gchar* input_port)
{
    memcpy(handler->endpoint.human_interface_ip,    input_ip,   strlen(input_ip));
    memcpy(handler->endpoint.human_interface_port,  input_port, strlen(input_port));
}


/**
 * @brief 
 * 
 * @param data 
 * @return gpointer 
 */
gpointer            gamepad_thread_func             (gpointer data);





void
trigger_capture_input_event(RemoteUdp* app)
{
    InputHandler* handler = remote_app_get_hid_handler(app);

#ifdef G_OS_WIN32
    handler->gamepad_thread = g_thread_new("gamepad thread", 
        (GThreadFunc)gamepad_thread_func, app);
#endif
}






struct _HidInput
{
    gdouble x_pos;
    gdouble y_pos;

    gdouble delta_x;
    gdouble delta_y;

    gint mouse_code;
    gboolean key_is_up;
    gint keyboard_code;

    gint wheel_dY;

    Win32Opcode opcode;
    gboolean relative;

    JsonObject* json;
};


static void
hid_data_channel_send(gchar* data,
                      RemoteUdp* remote)
{
    static gchar hid_url[50] = {0};
    static gboolean initialized = FALSE;
    const gchar* http_aliases[] = { "http", NULL };
    SoupSession* session = soup_session_new_with_options(
            SOUP_SESSION_SSL_STRICT, FALSE,
            SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
            SOUP_SESSION_HTTPS_ALIASES, http_aliases, NULL);

    if(!initialized)
    {
        InputHandler* handler = remote_app_get_hid_handler(remote);
        GString* url= g_string_new("http://");
        g_string_append(url,handler->endpoint.human_interface_ip);
        g_string_append(url,":");
        g_string_append(url,handler->endpoint.human_interface_port);
        g_string_append(url,"/hid");
        memcpy(hid_url,url->str,url->len);
    }

    SoupMessage* message = soup_message_new(SOUP_METHOD_POST,hid_url);
    soup_message_set_request(message,"application/json",SOUP_MEMORY_COPY,data,strlen(data));
    soup_session_send_async(session,message,NULL,NULL,NULL);

    if(DEVELOPMENT_ENVIRONMENT)
        g_print("%s\n",data);
}


static void
send_mouse_signal(HidInput* input,
                         RemoteUdp* core)
{
    JsonObject* object = json_object_new();
    json_object_set_int_member(object,"Opcode",(gint)input->opcode);
    json_object_set_int_member(object,"MouseCode",(gint)input->mouse_code);
    if(input->relative)
    {
        json_object_set_int_member(object,"dX",(gint)input->delta_x);
        json_object_set_int_member(object,"dY",(gint)input->delta_y);
    }
    else 
    {
        json_object_set_int_member(object,"dX",(gint)input->x_pos);
        json_object_set_int_member(object,"dY",(gint)input->y_pos);
    }
    hid_data_channel_send(get_string_from_json_object(object),core);
}

static void
send_mouse_wheel_signal(HidInput* input,
                         RemoteUdp* core)
{
    JsonObject* object = json_object_new();
    json_object_set_int_member(object,"Opcode",(gint)input->opcode);
    json_object_set_int_member(object,"WheeldY",(gint)input->wheel_dY);

    hid_data_channel_send(get_string_from_json_object(object),core);
}



static void
send_key_event(HidInput* input,
            RemoteUdp* core)
{
    JsonObject* object = json_object_new();
    json_object_set_int_member(object,"Opcode",(gint)input->opcode);
    json_object_set_boolean_member(object,"IsUp",input->key_is_up);
    json_object_set_int_member(object,"wVk",input->keyboard_code);

    hid_data_channel_send(get_string_from_json_object(object),core);
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
#ifdef G_OS_WIN32
    return (GetAsyncKeyState(key) & 0x8000) != 0;
#else
#endif
}





#ifdef G_OS_WIN32
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
reset_mouse(RemoteUdp* app)
{
    gint i = 0;
    while (!reset_mouse_array[i])
    {
        if(_keydown(reset_mouse_virtual_code[i]))
        {
            HidInput input = {0};
            input.opcode = MOUSERAW;
            input.mouse_code = reset_mouse_array[i];
            send_mouse_signal(&input,app);
        }
        i++;
    }
}




static void
send_gamepad_signal(HARDWAREINPUT input,
                    RemoteUdp* core)
{
    JsonObject* object = json_object_new();
    json_object_set_int_member(object,"Opcode",GAMEPAD_IN);
    json_object_set_int_member(object,"uMsg",(gint)input.uMsg);
    json_object_set_int_member(object,"wParamH",(gint)input.wParamH);
    json_object_set_int_member(object,"wParamL",(gint)input.wParamL);

    hid_data_channel_send(get_string_from_json_object(object),core);
}
#else
static gint reset_key_array[10] = 
{
    0,
};
#endif

void
reset_key(RemoteUdp* app)
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
            send_key_event(&input,app);
        }
        i++;
    }
}









void
toggle_key_capturing(RemoteUdp* app, gboolean is_true)
{
    InputHandler* handler = remote_app_get_hid_handler(app);
    handler->capturing = is_true;
}





/**
 * @brief 
 * parse human interface event 
 * @param input Hid structure to parse
 * @param core remote app
 */
static void
parse_hid_event(HidInput* input, 
                RemoteUdp* core)
{
    switch((gint)input->opcode)
    {
        case MOUSERAW:
            send_mouse_signal(input,core);
            break;
        case MOUSE_WHEEL:
            send_mouse_wheel_signal(input,core);
            break;
        case KEYRAW:
            send_key_event(input,core);
            break;
        default:
            return;
    }
}


#ifndef WIN32_HID_CAPTURE


#include <gst/video/navigation.h>


gboolean      
handle_navigator(GstEvent *event, 
                RemoteUdp* core)
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
            gst_navigation_event_parse_mouse_move_event(event,&(navigation->x_pos),&(navigation->y_pos));
            navigation->opcode = MOUSE_MOVE;
            break; 
        // case GST_NAVIGATION_EVENT_MOUSE_SCROLL: 
        //     gst_navigation_event_parse_mouse_scroll_event(event,&(navigation->x_pos),&(navigation->y_pos),&(navigation->delta_x),&(navigation->delta_y));
        //     navigation->opcode = MOUSE_WHEEL;
        //     break; 
        case GST_NAVIGATION_EVENT_MOUSE_BUTTON_PRESS: 
            gst_navigation_event_parse_mouse_button_event(event,&(navigation->mouse_code),&(navigation->x_pos),&(navigation->y_pos));
            navigation->opcode = MOUSE_DOWN;
            break; 
        case GST_NAVIGATION_EVENT_MOUSE_BUTTON_RELEASE: 
            gst_navigation_event_parse_mouse_button_event(event,&(navigation->mouse_code),&(navigation->x_pos),&(navigation->y_pos));
            navigation->opcode = MOUSE_UP;
            break; 
        default:
            break;
    }
    parse_hid_event(navigation,core);
    free(navigation);
}
#else


/**
 * @brief 
 * 
 * @param data 
 * @return gpointer 
 */
static gpointer 
gamepad_thread_func(gpointer data)
{
    RemoteUdp* app = (RemoteUdp*)data;

    XINPUT_STATE state, prevstate;
    memset(&state, 0, sizeof(XINPUT_STATE));
    memset(&prevstate, 0, sizeof(XINPUT_STATE));
    DWORD dwResult = XInputGetState(0, &prevstate);

    if (dwResult == ERROR_SUCCESS)
    {
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
    else
    {
        g_printerr("Cannot detect gamepad");
    }
}

void
send_gamepad_vibration(XINPUT_VIBRATION vibration)
{
    XInputSetState(0, &vibration);
}



static void
handle_window_keyboard(RAWKEYBOARD input,
                        HidInput* navigation)
{
    // https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawkeyboard
    gint VKey  = input.VKey;
    gint Message= input.Message;

    navigation->opcode = KEYRAW;
    // https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawkeyboard
    navigation->key_is_up = input.Flags;
    navigation->keyboard_code = VKey;
}

static void
handle_window_hid(RAWHID input,
                HidInput* navigation)
{
    gchar* buffer = input.bRawData;

    for(gint i = 1; i < input.dwCount + 1; i++)
    {
        GBytes* bytes = g_bytes_new(buffer+((i-1)*input.dwSizeHid),input.dwSizeHid);
        // handle byte here
    }
}

void 
handle_message_window_proc(HWND hwnd, 
                            UINT message, 
                            WPARAM wParam, 
                            LPARAM lParam)
{
    if(!HID_handler.capturing)
        return;

    gint KeyIndex;
    HidInput* navigation = malloc(sizeof(HidInput));

    guint dwSize;
    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
    gpointer buffer = malloc(dwSize);
    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) 
        return;


    RAWINPUT* raw_input = (RAWINPUT*) buffer;
    if (raw_input->header.dwType == RIM_TYPEKEYBOARD) 
    {
        handle_window_keyboard(raw_input->data.keyboard,navigation); 
    }
    

    parse_hid_event(navigation,HID_handler.app);
    free(navigation);
}

void
handle_window_mouse_relative(gint mouse_code,
                          gint delta_X,
                          gint delta_Y,
                          RemoteUdp* app)
{
    // reduce mouse move signal by filter unactive mouse
    if(mouse_code == WM_MOUSEMOVE)
    {
        if(!delta_X && !delta_Y)
        {
            return;
        }
    }


    HidInput* navigation = malloc(sizeof(HidInput));
    memset(navigation,0,sizeof(HidInput));
    navigation->opcode = MOUSERAW;
    navigation->relative = TRUE;
    navigation->mouse_code = mouse_code;
    navigation->delta_x = delta_X;
    navigation->delta_y = delta_Y;
    parse_hid_event(navigation,HID_handler.app);
    free(navigation);
}


void
handle_window_wheel(gint isup,
                    RemoteUdp* app)
{
    HidInput* navigation = malloc(sizeof(HidInput));
    memset(navigation,0,sizeof(HidInput));
    navigation->opcode = MOUSE_WHEEL;
    navigation->wheel_dY = isup? 120 : -120;
    parse_hid_event(navigation,HID_handler.app);
    free(navigation);
}

#endif