/**
 * @file remote-udp-gui.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <remote-udp-gui.h>
#include <remote-udp-type.h>
#include <remote-udp-pipeline.h>
#include <remote-udp-input.h>
#include <remote-udp.h>

#include <glib.h>

#include <gst/video/videooverlay.h>
#include <gst/video/gstvideosink.h>
#include <glib-2.0/glib.h>
#include <gst/video/videooverlay.h>
#include <gst/video/gstvideosink.h>
#include <gst/video/videooverlay.h>

#include <gtk/gtk.h>

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>  // for GDK_WINDOW_XID
#endif



struct _GUI
{
    /**
     * @brief 
     * reference to remote app
     */
    RemoteUdp *app;

#ifdef G_OS_WIN32
    /**
     * @brief 
     * win32 window for display video
     */
    HWND window;

    /**
     * @brief 
     * revious window style
     */
    LONG prev_style;

    /**
     * @brief 
     * previous rectangle size, use for setup fullscreen mode
     */
    RECT prev_rect;

    /**
     * @brief 
     * window retangle, size and position of window
     */
    RECT wr;

    /**
     * @brief 
     * IO channel for remote app
     */
    GIOChannel *msg_io_channel;
#else 
    GtkApplication *gtk_app;

    guintptr window_handle;

    GtkWidget *video_window;

    GtkWidget *app_window;
#endif

    /**
     * @brief 
     * gst video sink element
     */
    GstElement* sink_element;

    GstElement* pipeline;

    /**
     * @brief 
     * fullscreen mode on or off
     */
    gboolean fullscreen;


    gboolean disable_client_cursor;
};

static GUI _gui = {0};

/**
 * @brief Set the up window object
 * 
 * @param gui 
 */
void                        set_up_window               (GUI* gui);

/**
 * @brief 
 * 
 */
void                        handle_fullscreen_hotkey    ();

#ifndef G_OS_WIN32
/**
 * @brief 
 * 
 */
static void                 video_widget_realize_cb     (GtkWidget * widget, 
                                                         gpointer data);
#endif

static void
activate (GtkApplication* app,
          gpointer        user_data)
{
    GUI* gui = (GUI*) user_data;

    gui->app_window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (gui->app_window), "Window");
    gtk_window_set_default_size (GTK_WINDOW (gui->app_window), 200, 200);

    gui->video_window = gtk_drawing_area_new();
    g_signal_connect(gui->video_window,"realize",G_CALLBACK(video_widget_realize_cb),gui);

    gtk_widget_set_double_buffered (gui->video_window, FALSE);
    gtk_container_add (GTK_CONTAINER (gui->app_window), gui->video_window);
    gtk_widget_show_all (gui->app_window);

    gtk_widget_realize (gui->video_window);
}

gpointer
run_application(gpointer data)
{
    gint status;
    status = g_application_run (G_APPLICATION (data), 0, NULL);
    g_object_unref (data);
}

GUI*
init_remote_app_gui(RemoteUdp *app)
{
    memset(&_gui,0,sizeof(GUI));
    _gui.app = app;
#ifdef G_OS_WIN32
    _gui.wr = { 0, 0, 320, 240 };
    set_up_window(&_gui);
#else
    GtkApplication* gtk_app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (gtk_app, "activate", G_CALLBACK (activate), &_gui);
    g_thread_new("gui",run_application,gtk_app);
#endif
    return &_gui;
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
    wc.style = CS_HREDRAW | CS_VREDRAW; //// ????
    wc.lpfnWndProc = (WNDPROC)window_proc;
    wc.hInstance = hinstance;
    wc.hCursor = LoadCursor(NULL, IDC_NO);
    wc.lpszClassName = "GstWIN32VideoOverlay";
    RegisterClassEx(&wc);

    AdjustWindowRect (&(gui->wr), WS_OVERLAPPEDWINDOW, FALSE);
    gui->window = CreateWindowEx(0, wc.lpszClassName,
                          "Thinkremote",
                          WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          gui->wr.right  - gui->wr.left, 
                          gui->wr.bottom - gui->wr.top, 
                          (HWND)NULL, (HMENU)NULL,
                          hinstance, NULL);
    gui->msg_io_channel = g_io_channel_win32_new_messages (0);
    g_io_add_watch (gui->msg_io_channel, G_IO_IN, msg_cb, NULL);

	{
		RAWINPUTDEVICE devices[1];
		devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		devices[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
		devices[0].dwFlags = RIDEV_NOLEGACY | RIDEV_NOHOTKEYS;
		devices[0].hwndTarget = _gui.window;

		BOOL registered = RegisterRawInputDevices(devices, ARRAYSIZE(devices),
			sizeof(RAWINPUTDEVICE));
		if(registered == FALSE)
		{
			g_printerr("Registering keyboard and/or mouse as Raw Input devices failed!");
		}
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
    RemoteUdp * remote = (RemoteUdp*) user_data;    
    GUI* gui = remote_app_get_gui(remote);
    GstElement *pipeline = GST_ELEMENT (gui->pipeline);

    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_ASYNC_DONE:
            break;
    }
    return TRUE;
}




gpointer
setup_video_overlay(GstElement* videosink, 
                    GstElement* pipeline, 
                    RemoteUdp* app)
{
    GUI* gui = remote_app_get_gui(app);
    gui->sink_element = videosink;
    gui->pipeline = pipeline;

    /* prepare the pipeline */
    gst_bus_add_watch (GST_ELEMENT_BUS (pipeline), bus_msg, app);
    gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (gui->sink_element), 
        (guintptr) gui->window);
    ShowWindow (gui->window, SW_SHOW);
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
    {
        return FALSE;
    }

    dev_mode.dmSize = sizeof(dev_mode);
    dev_mode.dmDriverExtra = sizeof(POINTL);
    dev_mode.dmFields = DM_POSITION;
    if (!EnumDisplaySettings(monitor_info.szDevice, ENUM_CURRENT_SETTINGS, &dev_mode))
    {
        return FALSE;
    }

    SetRect(rect, 0, 0, dev_mode.dmPelsWidth, dev_mode.dmPelsHeight);

    return TRUE;
}








void 
switch_fullscreen_mode(GUI* gui)
{
    long _prev_style;

    gui->fullscreen = !gui->fullscreen;

    if (!gui->fullscreen)
    {
        /* Restore the window's attributes and size */
        SetWindowLong(gui->window, GWL_STYLE, gui->prev_style);

        SetWindowPos(gui->window, HWND_NOTOPMOST,
                    gui->prev_rect.left,
                    gui->prev_rect.top,
                    1556 - gui->prev_rect.left,
                    884 - gui->prev_rect.top, SWP_FRAMECHANGED | SWP_NOACTIVATE);

        ShowWindow(gui->window, SW_NORMAL);
    }
    else
    {
        RECT fullscreen_rect;

        /* show window before change style */
        ShowWindow(gui->window, SW_SHOW);

        /* Save the old window rect so we can restore it when exiting
        * fullscreen mode */
        GetWindowRect(gui->window, &(gui->prev_rect));
        gui->prev_style = GetWindowLong(gui->window, GWL_STYLE);

        if (!get_monitor_size(&fullscreen_rect, gui->window))
        {
          g_warning("Couldn't get monitor size");

          gui->fullscreen = !gui->fullscreen;
          return;
        }

        /* Make the window borderless so that the client area can fill the screen */
        _prev_style = gui->prev_style;
        SetWindowLong(gui->window, GWL_STYLE,
                      _prev_style &
                          ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU |
                            WS_THICKFRAME));
        gui->prev_style = _prev_style;
        SetWindowPos(gui->window, HWND_NOTOPMOST,
                    fullscreen_rect.left,
                    fullscreen_rect.top,
                    fullscreen_rect.right,
                    fullscreen_rect.bottom, SWP_FRAMECHANGED | SWP_NOACTIVATE);
        ShowWindow(gui->window, SW_MAXIMIZE);
    }
}



/**
 * @brief 
 * 
 * @param app 
 * @param x 
 * @param y 
 * @param width 
 * @param height 
 * @return gboolean 
 */
static gboolean
adjust_video_position(RemoteUdp* app, 
                      gint x, 
                      gint y, 
                      gint width, 
                      gint height)
{
    GUI* gui = remote_app_get_gui(app);
    gst_video_overlay_set_render_rectangle(GST_VIDEO_OVERLAY(gui->sink_element), 
                                            x, 
                                            y, 
                                            width, 
                                            height);
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
 * center mouse to center of the screee
 * @param gui 
 * @return RECT new position of the screen 
 */
POINT
center_mouse_position(GUI* gui)
{
    POINT pt;
    RECT rect;
    GetWindowRect(gui->window,&rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    pt.x = width /2;
    pt.y = height /2;
    ClientToScreen(gui->window, &pt);
    SetCursorPos(pt.x,pt.y);
    ScreenToClient(gui->window,&pt);
    return pt; 
}


#define F_KEY  0x50
#define W_KEY  0x57



/**
 * @brief 
 * 
 */
void
handle_fullscreen_hotkey()
{
    if(_gui.disable_client_cursor)
    {
        enable_client_cursor();
        toggle_key_capturing(_gui.app,FALSE);
        switch_fullscreen_mode(&_gui);
    }
    else
    {
        disable_client_cursor();
        switch_fullscreen_mode(&_gui);
        toggle_key_capturing(_gui.app,TRUE);

        /**
         * @brief 
         * reset mouse and keyboard to prevent key stuck
         */
        reset_key(_gui.app);
        reset_mouse(_gui.app);
    }
}


void
handle_user_shortcut()
{
    if (_keydown(VK_SHIFT))
    {
        if (_keydown(VK_MENU))
        {
            if (_keydown(VK_CONTROL))
            {
                /**
                 * @brief 
                 * handle mouse lock key
                 */
                if (_keydown(F_KEY))
                {
                    handle_fullscreen_hotkey();
                }

                /**
                 * @brief 
                 * handle reset video stream 
                 */
                if (_keydown(W_KEY))
                {
                    remote_app_reset(_gui.app);
                }
            }
        }
    }
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
    if (message == WM_DESTROY) 
    {
        remote_app_finalize(_gui.app,NULL);
    } 
    else if (message == WM_INPUT)
    {
        handle_user_shortcut();
        handle_message_window_proc(hWnd, message, wParam, lParam );
    }
    else if (message == WM_MOUSEMOVE    ||
             message == WM_LBUTTONDOWN	||
             message == WM_LBUTTONUP	||
             message == WM_MBUTTONDOWN	||
             message == WM_MBUTTONUP	||
             message == WM_RBUTTONDOWN	||
             message == WM_RBUTTONUP	||
             message == WM_XBUTTONDOWN	||
             message == WM_XBUTTONUP)
    {
        gint x = LOWORD(lParam);
		gint y = HIWORD(lParam);

        if(_gui.disable_client_cursor)
        {
            POINT pt = center_mouse_position(&_gui);
            handle_window_mouse_relative(message,
                x-pt.x,
                y-pt.y,
                _gui.app);
        }
    }
    else if (message == WM_MOUSEWHEEL)
    {		
		gboolean up = (GET_WHEEL_DELTA_WPARAM(wParam) > 0);

        if(_gui.disable_client_cursor)
            center_mouse_position(&_gui);

        handle_window_wheel(up,_gui.app);
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}



/**
 * @brief 
 * 
 * @param gui 
 */
static void 
adjust_window(GUI* gui)
{
    AdjustWindowRect(&(gui->wr), WS_OVERLAPPEDWINDOW, FALSE);
}


void
disable_client_cursor()
{
    _gui.disable_client_cursor = TRUE;
    ShowCursor(FALSE);
}

void
enable_client_cursor()
{
    _gui.disable_client_cursor = FALSE;
    ShowCursor(TRUE);
}


void
gui_terminate(GUI* gui)
{
    DestroyWindow(gui->window);
}


#else

static GstBusSyncReply
bus_sync_handler (GstBus * bus, 
                  GstMessage * message, 
                  gpointer user_data)
{
    RemoteUdp* udp = (RemoteUdp*) user_data;
    GUI* gui = remote_app_get_gui(udp);

    // ignore anything but 'prepare-window-handle' element messages
    if (!gst_is_video_overlay_prepare_window_handle_message (message))
        return GST_BUS_PASS;

    if (gui->window_handle != 0) 
    {
        GstVideoOverlay *overlay;
        // GST_MESSAGE_SRC (message) will be the video sink element
        overlay = GST_VIDEO_OVERLAY (GST_MESSAGE_SRC (message));
        gst_video_overlay_set_window_handle (overlay, gui->window_handle);
    } 
    else 
    {
        g_warning ("Should have obtained video_window_handle by now!");
    }

    gst_message_unref (message);
    return GST_BUS_DROP;
}

static void
video_widget_realize_cb (GtkWidget * widget, 
                         gpointer data)
{
    GUI* gui = (GUI*)data;
    gulong xid = GDK_WINDOW_XID (gtk_widget_get_window (gui->video_window));
    gui->window_handle = xid;
}



gpointer
setup_video_overlay(GstElement* videosink,
                    GstElement* pipeline, 
                    RemoteUdp* app)
{
    GstBus* bus;
    GUI* gui = remote_app_get_gui(app);

    /* prepare the pipeline */
    gst_object_ref_sink (videosink);

    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    gst_bus_set_sync_handler (bus, (GstBusSyncHandler) bus_sync_handler, NULL, NULL);
    gst_object_unref (bus);
}

void
gui_terminate(GUI* gui)
{
    g_application_quit(gui->gtk_app);
}
#endif