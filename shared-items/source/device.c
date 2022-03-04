#include <glib-2.0/glib.h>
#include <gst/gst.h>

#include <device.h>


struct _MediaDevice
{
    gchar sound_capture_device_id[1000];
    gchar sound_output_device_id[1000];

    gchar backup_sound_capture_device_id[1000];
    gchar backup_sound_output_device_id[1000];

    guint64 monitor_handle;
    guint64 backup_monitor_handle;

    gchar monitor_name[100];
    gchar backup_monitor_name[100];
};


static void
device_foreach(GstDevice* device, 
               gpointer data)
{
    MediaDevice* source = (MediaDevice*) data;

    gchar* name = gst_device_get_display_name(device);

    GString* string = g_string_new("Probing media device: ");
    g_string_append(string,name);
    worker_log_output(g_string_free(string,FALSE));
                
    gchar* class = gst_device_get_device_class(device);
    GstCaps* cap = gst_device_get_caps(device);
    GstStructure* cap_structure = gst_caps_get_structure (cap, 0);
    GstStructure* device_structure = gst_device_get_properties(device);
    gchar* cap_name = gst_structure_get_name (cap_structure);
    gchar* api = gst_structure_get_string(device_structure,"device.api");

    
    if(!g_strcmp0(api,"wasapi2"))
    {
        gboolean is_default;
        gchar* id;

        id = gst_structure_get_string(device_structure,"device.strid");
        id = id ? id : gst_structure_get_string(device_structure,"device.id");
        gst_structure_get_boolean(device_structure,"device.default",&is_default);

        if(!g_strcmp0(class,"Audio/Source") &&
           !g_strcmp0(cap_name,"audio/x-raw"))
        {
            if(g_str_has_prefix(name,"CABLE Input"))
                memcpy(source->sound_output_device_id,id,strlen(id));
            else if(is_default && !g_str_has_prefix(name,"Default Audio Capture"))
                memcpy(source->backup_sound_output_device_id,id,strlen(id));
        }

        if(!g_strcmp0(class,"Audio/Sink") &&
           !g_strcmp0(cap_name,"audio/x-raw"))
        {
            if(g_str_has_prefix(name,"CABLE"))
                memcpy(source->sound_capture_device_id,id,strlen(id));
            else if(is_default)
                memcpy(source->backup_sound_capture_device_id,id,strlen(id));
        }
    }


    if(!g_strcmp0(class,"Source/Monitor") && 
       !g_strcmp0(api,"d3d11"))
    {
        gboolean is_primary;
        guint64 id; 
        gst_structure_get_uint64(device_structure,"device.hmonitor",&id);
        gst_structure_get_boolean(device_structure,"device.primary",&is_primary);
        gchar* display_name = gst_structure_get_string(device_structure,"device.name");
        if(!g_strcmp0(name,"Linux FHD"))
        {
            source->monitor_handle = id;
            memcpy(source->monitor_name,display_name,strlen(display_name));
        }
        else if (is_primary)
        {
            source->backup_monitor_handle = id;
            memcpy(source->backup_monitor_name,display_name,strlen(display_name));
        }
    }

    gst_caps_unref(cap);
    g_object_unref(device);
}


MediaDevice*
init_media_device_source()
{
    MediaDevice* device = malloc(sizeof(MediaDevice));
    memset(device,0,sizeof(MediaDevice));
    return device;
}

void
set_media_device(MediaDevice* device)
{
    GstDeviceMonitor* monitor = gst_device_monitor_new();
    worker_log_output("Searching for available device");
    if(!gst_device_monitor_start(monitor)) {
        worker_log_output("WARNING: Monitor couldn't started!!\n");
        return NULL;
    }

    GList* device_list = gst_device_monitor_get_devices(monitor);
    g_list_foreach(device_list,(GFunc)device_foreach,device);
}


gchar* 
get_audio_source(MediaDevice* source)
{
    return source->sound_capture_device_id ? source->sound_capture_device_id : source->backup_sound_capture_device_id;
}

guint64 
get_video_source(MediaDevice* source)
{
    return source->monitor_handle ? source->monitor_handle : source->backup_monitor_handle;
}

gchar*
get_video_source_name(MediaDevice* source)
{
    return source->monitor_handle ? source->monitor_name : source->backup_monitor_name;
}