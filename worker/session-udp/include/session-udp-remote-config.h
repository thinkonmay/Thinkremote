/// <summary>
/// @file session-udp-remote-config.h
/// @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
/// </summary>
/// @version 1.0
/// @date 2021-09-05
/// 
/// @copyright Copyright (c) 2021
/// 
#ifndef __SESSION_CORE_REMOTE_CONFIG_H__
#define __SESSION_CORE_REMOTE_CONFIG_H__
#include "session-udp-type.h"
#include "session-udp.h"
#include <gst/gst.h>
#include <qoe.h>





/**
 * @brief 
 * initialize StreamConfig for the stream
 * @return StreamConfig* 
 */
StreamConfig*			qoe_initialize						();

/**
 * @brief 
 * setup QoS with given metric
 * @param qoe qoe 
 * @param screen_width  screen width that will be capture by session core, current screen size will be automatically adjusted to fit with client need
 * @param screen_height  screen height that will be capture by session core, current screen size will be automatically adjusted to fit with client need
 * @param audio_codec audio codec that will be used for the stream
 * @param video_codec video codec that will be used for the stream
 * @param mode qoe mode that will be used during the stream
 */
void 			qoe_setup							(StreamConfig* qoe,
													gint screen_width,
													gint screen_height,
													Codec audio_codec,
													Codec video_codec,
													QoEMode mode);


/**
 * @brief 
 * get audio codec of the stream
 * @param qoe 
 * @return Codec 
 */
Codec			qoe_get_audio_codec					(StreamConfig* qoe);

/**
 * @brief 
 * get video codec of the stream
 * @param qoe 
 * @return Codec 
 */
Codec			qoe_get_video_codec					(StreamConfig* qoe);


/**
 * @brief 
 * get screen height of current session  
 * @param qoe 
 * @return gint 
 */
gint			qoe_get_screen_height				(StreamConfig* qoe);



/**
 * @brief 
 * get screen width of current session
 * @param qoe 
 * @return gint 
 */
gint			qoe_get_screen_width				(StreamConfig* qoe);


/**
 * @brief 
 * get video bitrate will be used throughout the video stream 
 * @param qoe 
 * @return gint video bitrate in kbit/s 
 */
gint 			qoe_get_video_bitrate				(StreamConfig* qoe);

#endif