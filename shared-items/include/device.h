/**
 * @file device.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2022-03-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <glib-2.0/glib.h>

typedef struct _MediaDevice                     MediaDevice;

/**
 * @brief Get the media device source object
 * 
 * @return MediaDevice* 
 */
MediaDevice*    init_media_device_source        ();

/**
 * @brief Set the media device object
 * 
 * @param device 
 */
void            set_media_device                (MediaDevice* device);

/**
 * @brief 
 * 
 */
guint64          get_video_source                (MediaDevice* source);

/**
 * @brief 
 * 
 */
gchar*          get_audio_source                (MediaDevice* source);

/**
 * @brief 
 * 
 */
gchar*          get_video_source_name           (MediaDevice* source);

/**
 * @brief Get the local ip object
 * 
 * @return gchar* 
 */
gchar*          get_local_ip                    ();

#endif