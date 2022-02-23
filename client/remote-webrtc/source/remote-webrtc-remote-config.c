/**
 * @file remote-webrtc-remote-config.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <remote-webrtc-remote-config.h>
#include <remote-webrtc-type.h>
#include <remote-webrtc-pipeline.h>

#include <enum.h>

#include <gst/gst.h>
#include <glib-2.0/glib.h>


#include <stdio.h>




struct _QoE
{
	/*codec audio*/
	Codec codec_audio;
	Codec codec_video;

};


QoE*
qoe_initialize()
{
	QoE* qoe = malloc(sizeof(QoE));
    memset(qoe,0,sizeof(QoE));
	return qoe;
}



void
qoe_setup(QoE* qoe,
		  Codec audio_codec,
		  Codec video_codec)
{
	qoe->codec_audio = audio_codec;
	qoe->codec_video = video_codec;
}

Codec
qoe_get_audio_codec(QoE* qoe)
{
	return qoe->codec_audio;
}

Codec
qoe_get_video_codec(QoE* qoe)
{
	return qoe->codec_video;
}
