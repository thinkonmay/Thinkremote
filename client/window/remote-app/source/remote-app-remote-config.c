/**
 * @file remote-app-remote-config.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <remote-app-remote-config.h>
#include <remote-app-type.h>
#include <remote-app-pipeline.h>

#include <qoe.h>

#include <gst/gst.h>
#include <glib-2.0/glib.h>
#include <gst/controller/gstinterpolationcontrolsource.h>
#include <gst/controller/gstdirectcontrolbinding.h>


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
