/**
 * @file remote-udp-remote-config.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <remote-udp-type.h>
#include "remote-udp.h"
#include <gst/gst.h>
#include <qoe.h>





/**
 * @brief 
 * initialize qoe metric collection
 * @return RemoteConfig* 
 */
RemoteConfig*			qoe_initialize						();

/**
 * @brief 
 * setup qoe 
 * @param qoe 
 * @param audio_codec 
 * @param video_codec 
 */
void			qoe_setup							(RemoteConfig* qoe,
		  											Codec audio_codec,
		  											Codec video_codec);


/**
 * @brief 
 * update QoS
 * @param core 
 * @param time 
 * @param framerate 
 * @param audio_latency 
 * @param video_latency 
 * @param audio_bitrate 
 * @param video_bitrate 
 * @param bandwidth 
 * @param packets_lost 
 */
void			qoe_update_quality					(RemoteUdp* core,
													 gint time,
													 gint framerate,
													 gint audio_latency,
													 gint video_latency,
													 gint audio_bitrate,
													 gint video_bitrate,
												     gint bandwidth,
													 gint packets_lost);

/**
 * @brief 
 * get audio codec of the stream
 * @param qoe 
 * @return Codec 
 */
Codec			qoe_get_audio_codec					(RemoteConfig* qoe);

/**
 * @brief 
 * get video codec of the stream
 * @param qoe 
 * @return Codec 
 */
Codec			qoe_get_video_codec					(RemoteConfig* qoe);