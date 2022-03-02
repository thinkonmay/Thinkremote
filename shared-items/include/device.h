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
MediaDevice*    get_media_device_source         ();

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

#endif