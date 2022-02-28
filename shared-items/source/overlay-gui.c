/**
 * @file remote-webrtc-gui.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <overlay-gui.h>


#include <key-convert.h>
#include <capture-key.h>
#include <shortcut.h>
#include <global-var.h>

#include <logging.h>
#include <json-handler.h>

#include <glib-2.0/glib.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <gst/video/gstvideosink.h>

#define BORDER_SIZE   30



struct _GUI
{
    /**
     * @brief 
     * reference to remote app
     */
    gpointer app;

    /**
     * @brief 
     * 
     */
    InputHandler* handler;

    /**
     * @brief 
     * win32 window for display video
     */
    HWND window;

    /**
     * @brief 
     * gst video sink element
     */
    GstElement* sink_element;

    /**
     * @brief 
     * 
     */
    GstCaps* stream_cap;







    /**
     * @brief 
     * previous rectangle size, use for setup fullscreen mode
     */
    RECT window_position;

    /**
     * @brief 
     * 
     */
    gboolean fullscreen;

    /**
     * @brief 
     * 
     */
    gboolean client_pointer;

    /**
     * @brief 
     * 
     */
    gfloat scale_ratio;
};


/**
 * @brief Set the up window object
 * 
 * @param gui 
 */
void                        set_up_window            (GUI* gui);

/**
 * @brief 
 * 
 * @param gui 
 */
void                        handle_fullscreen_hotkey (GUI* gui);

/**
 * @brief 
 * 
 */
void                        toggle_client_cursor     ();



/**
 * @brief Get the remote resolution object
 * 
 * @param gui 
 * @param size 
 */
void
get_remote_resolution(GUI* gui,
                      RECT* size)
{
    GstStructure* structure = gst_caps_get_structure(gui->stream_cap,0);
    gst_structure_get_int(structure,"height",&size->bottom);
    gst_structure_get_int(structure,"width", &size->right);
    size->left = 0;
    size->top  = 0;
}


static GUI _gui = {0};

gboolean
is_hover_window()
{
    POINT pos;
    GetCursorPos(&pos);
    return((pos.x < _gui.window_position.right) &&
           (pos.x > _gui.window_position.left) &&
           (pos.y > ( _gui.window_position.top + BORDER_SIZE) ) &&
           (pos.y < _gui.window_position.bottom));
}

static void
add_gui_shortcuts(Shortcut* shortcuts)
{
    gint key_list[10] = {0};
    key_list[0] = F_KEY;
    key_list[1] = VK_SHIFT;
    key_list[2] = VK_CONTROL;
    key_list[3] = VK_MENU;

	add_new_shortcut_to_list(shortcuts,key_list,
        RELOAD_STREAM,handle_fullscreen_hotkey,&_gui);
}

GUI*
init_remote_app_gui(gpointer app,
                    Shortcut* shortcuts,
                    HIDHandleFunction handler)
{
    GUI* gui = &_gui;
    add_gui_shortcuts(shortcuts);
    gui->handler = init_input_capture_system(handler,shortcuts,app);
    gui->app = app;
    gui->window_position = (RECT){0,0,1920,1080};
    set_up_window(gui);
    return gui;
}

#ifdef G_OS_WIN32
#include <windows.h>
#include <WinUser.h>
#include <libloaderapi.h>
#include <Xinput.h>
#include <hidusage.h>



/**
 * @brief 
 * window procedure function
 * @param hWnd 
 * @param message 
 * @param wParam 
 * @param lParam 
 * @return LRESULT 
 */
static LRESULT CALLBACK       window_proc             (HWND hWnd, 
                                                      UINT message, 
                                                      WPARAM wParam, 
                                                      LPARAM lParam);

/**
 * @brief 
 * 
 * @param source 
 * @param condition 
 * @param data 
 * @return gboolean 
 */
static gboolean
msg_cb (GIOChannel * source, 
        GIOCondition condition, 
        gpointer data)
{
  MSG msg;

  if (!PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
    return G_SOURCE_CONTINUE;

  TranslateMessage (&msg);
  DispatchMessage (&msg);

  return G_SOURCE_CONTINUE;
}

/**
 * @brief Set the up window object
 * setup window 
 * @param gui 
 * @return HWND 
 */
static void
set_up_window(GUI* gui)
{
    HINSTANCE hinstance = GetModuleHandle(NULL);

    WNDCLASSEX wc = { 0, };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW; 
    wc.lpfnWndProc = (WNDPROC)window_proc;
    wc.hInstance = hinstance;
    wc.hCursor = LoadCursor(NULL, IDC_NO);

    wc.lpszClassName = "GstWIN32VideoOverlay";
    RegisterClassEx(&wc);

    AdjustWindowRect (&(gui->window_position), WS_OVERLAPPEDWINDOW , FALSE);
    gui->window = CreateWindowEx(0, wc.lpszClassName,
                          "Thinkremote",
                          WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          gui->window_position.right  - gui->window_position.left, 
                          gui->window_position.bottom - gui->window_position.top, 
                          (HWND)NULL, (HMENU)NULL,
                          hinstance, NULL);
    GIOChannel* msg_io_channel = g_io_channel_win32_new_messages (0);
    g_io_add_watch (msg_io_channel, G_IO_IN, msg_cb, NULL);

	{
		RAWINPUTDEVICE devices[1];
		devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		devices[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
		devices[0].dwFlags = RIDEV_NOLEGACY | RIDEV_NOHOTKEYS;
		devices[0].hwndTarget = gui->window;

		BOOL registered = RegisterRawInputDevices(devices, ARRAYSIZE(devices),
			sizeof(RAWINPUTDEVICE));

		if(registered == FALSE)
			worker_log_output("Registering keyboard and/or mouse as Raw Input devices failed!");
	}

	{
        RAWINPUTDEVICE Rid[1];
        Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC; 
        Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE; 
        Rid[0].dwFlags = RIDEV_INPUTSINK;   
        Rid[0].hwndTarget = gui->window;
        gboolean registered = RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

		if(registered == FALSE)
			worker_log_output("Registering keyboard and/or mouse as Raw Input devices failed!");
	}
}


/**
 * @brief 
 * handle message from gbus of gstreamer pipeline
 * @param bus 
 * @param msg 
 * @param user_data 
 * @return gboolean 
 */
static gboolean
bus_msg (GstBus * bus, 
        GstMessage * msg, 
        gpointer user_data)
{
  GstElement *pipeline = GST_ELEMENT (user_data);
  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_ASYNC_DONE:
      gst_element_set_state (pipeline, GST_STATE_PLAYING);
      break;
  }

  return TRUE;
}



void
adjust_window_size(GUI* gui)
{

    /* Restore the window's attributes and size */
    RECT stream;
    get_remote_resolution(gui,&stream);
    gfloat stream_ratio = ((gfloat) stream.right) / ((gfloat)stream.bottom);

    RECT rect;
    GetWindowRect(gui->window,&rect);
    rect.top = rect.top - BORDER_SIZE;

    gint width = rect.right - rect.left;
    gint height = rect.bottom - rect.top;
    gfloat window_ratio = ((gfloat) width) / ((gfloat)height);

    POINT center;
    center.x = ( rect.left + rect.right )/2;
    center.y = ( rect.top  + rect.bottom )/2;

    gint new_height = (stream_ratio > window_ratio) ? ( width  / stream_ratio ) : height;
    gint new_width  = (stream_ratio < window_ratio) ? ( height * stream_ratio ) : width;

    new_height = new_height + BORDER_SIZE;

    gui->scale_ratio = (gfloat)new_width / (gfloat)stream.right;
    
    gui->window_position.left =  center.x - (new_width / 2);
    gui->window_position.right = center.x + (new_width / 2);

    gui->window_position.top    = center.y - (new_height / 2);
    gui->window_position.bottom = center.y + (new_height / 2);

    if(gui->fullscreen)
        return;

    MoveWindow(gui->window, 
                gui->window_position.left,
                gui->window_position.top + BORDER_SIZE,
                new_width,
                new_height, 
                NULL);


}


gpointer
setup_video_overlay(GUI* gui,
                    GstCaps* caps,
                    GstElement* videosink, 
                    GstElement* pipeline)
{
    gui->stream_cap = caps;
    gui->sink_element = videosink;
    get_remote_resolution(gui,&gui->window_position);

    /* prepare the pipeline */
    gst_object_ref_sink (videosink);
    gst_bus_add_watch (GST_ELEMENT_BUS (pipeline), bus_msg, pipeline);

    adjust_window_size(gui);
    ShowWindow (gui->window, SW_SHOW);
    gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (videosink),
        (guintptr) gui->window);
    toggle_input_capture(gui->handler);
}




/**
 * @brief Get the monitor size object
 * 
 * @param rect 
 * @param hwnd 
 * @return gboolean 
 */
static gboolean
get_monitor_size(RECT *rect, 
                 HWND *hwnd)
{
    HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFOEX monitor_info;
    DEVMODE dev_mode;

    monitor_info.cbSize = sizeof(monitor_info);

    if (!GetMonitorInfo(monitor, (LPMONITORINFO)&monitor_info))
        return FALSE;

    dev_mode.dmSize = sizeof(dev_mode);
    dev_mode.dmDriverExtra = sizeof(POINTL);
    dev_mode.dmFields = DM_POSITION;

    if (!EnumDisplaySettings(monitor_info.szDevice, ENUM_CURRENT_SETTINGS, &dev_mode))
        return FALSE;

    SetRect(rect, 0, 0, dev_mode.dmPelsWidth, dev_mode.dmPelsHeight);

    return TRUE;
}








void 
switch_fullscreen_mode(GUI* gui)
{
    static glong style; 
    if (gui->fullscreen)
    {
        /* show window before change style */
        ShowWindow(gui->window, SW_SHOW);

        SetWindowLong(gui->window, GWL_STYLE, style);
        adjust_window_size(gui);
        ShowWindow(gui->window, SW_NORMAL);
    }
    else
    {
        /* show window before change style */
        ShowWindow(gui->window, SW_SHOW);

        RECT fullscreen_rect;
        if (!get_monitor_size(&fullscreen_rect, gui->window))
            return;

        style = GetWindowLong(gui->window, GWL_STYLE);
        SetWindowLong(gui->window, GWL_STYLE, style &
                          ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU |
                            WS_THICKFRAME));

        SetWindowPos(gui->window, HWND_NOTOPMOST,
                    fullscreen_rect.left,
                    fullscreen_rect.top,
                    fullscreen_rect.right,
                    fullscreen_rect.bottom, 
                    SWP_FRAMECHANGED | SWP_NOACTIVATE);

        ShowWindow(gui->window, SW_MAXIMIZE);
    }
    gui->fullscreen = !gui->fullscreen;
}











/**
 * @brief 
 * 
 */
void
handle_fullscreen_hotkey(GUI* gui)
{
    switch_fullscreen_mode(gui);
}



gpointer 
size_handle_thread(gpointer data)
{
    static gboolean sizing = FALSE;

    if (sizing)
        return;

    sizing = TRUE;
    Sleep(2000);
    adjust_window_size((GUI*) data);
    sizing = FALSE;
}



void
on_new_window_position(GUI* gui)
{
    GetWindowRect(gui->window,&gui->window_position);
}

/**
 * @brief 
 * window proc responsible for handling message from window HWND
 * @param hWnd 
 * @param message 
 * @param wParam 
 * @param lParam 
 * @return LRESULT 
 */
static LRESULT CALLBACK
window_proc(HWND hWnd, 
            UINT message, 
            WPARAM wParam, 
            LPARAM lParam)
{
    GUI* gui = &_gui;
    if (message == WM_DESTROY) 
        trigger_hotkey_by_opcode(EXIT);

    if (message == WM_INPUT)
        handle_message_window_proc(hWnd, message, wParam, lParam );

    if (message == WM_MOUSEWHEEL)
        handle_window_wheel(GET_WHEEL_DELTA_WPARAM(wParam) > 0);

    if (message == WM_LBUTTONDOWN  || message == WM_LBUTTONUP	||
        message == WM_MBUTTONDOWN  || message == WM_MBUTTONUP	||
        message == WM_RBUTTONDOWN  || message == WM_RBUTTONUP	||
        message == WM_XBUTTONDOWN  || message == WM_XBUTTONUP)
        handle_mouse_button(message);

    if (message == WM_SIZE)
        g_thread_new("handle resize",size_handle_thread,gui);

    if (message == WM_MOVE)
        on_new_window_position(gui);

    return DefWindowProc(hWnd, message, wParam, lParam);
}


#define CLIENT_CURSOR_OFFSET 0

void
gui_set_cursor_position(JsonObject* object)
{
    if(!is_hover_window())
        return;


    GUI* gui = &_gui;

    gint x = json_object_get_int_member(object,"X");
    gint y = json_object_get_int_member(object,"Y");

    POINT _position = {x,y};
    POINT* position = &_position;
    position->x = position->x * gui->scale_ratio;
    position->y = position->y * gui->scale_ratio;

    position->x = position->x + CLIENT_CURSOR_OFFSET;
    position->y = position->y + CLIENT_CURSOR_OFFSET;

    ClientToScreen(gui->window,position);
    SetCursorPos(position->x,position->y);
}


void
toggle_client_cursor(GUI* gui)
{
    ShowCursor(gui->client_pointer);
    trigger_hotkey_by_opcode(gui->client_pointer ? WORKER_POINTER_ON : WORKER_POINTER_OFF);
}



#endif