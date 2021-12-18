/**
 * @file session-core-remote-config.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <session-core-remote-config.h>
#include <session-core-type.h>
#include <session-core-pipeline.h>

#include <opcode.h>
#include <qoe.h>

#include <gst/gst.h>
#include <glib-2.0/glib.h>


#include <stdio.h>
#include <Windows.h>




struct _StreamConfig
{
	/**
	 * @brief 
	 * screen height be captured by session core
	 */
	gint screen_height;

	/**
	 * @brief 
	 * screen width becaptured by session core
	 */
	gint screen_width;

	/**
	 * @brief 
	 * qoe mode of the stream
	 */
	QoEMode mode;

	/**
	 * @brief 
	 * codec audio will be used throughout the stream
	 */
	Codec codec_audio;

	/**
	 * @brief 
	 * codec video will be used throughout the stream
	 */
	Codec codec_video;

};


StreamConfig*
qoe_initialize()
{
	StreamConfig* config = malloc(sizeof(StreamConfig));
	memset(config, 0, sizeof(StreamConfig));
	return config;
}


void
display_setting_get_and_set(gint* screen_width,
							gint* screen_height)
{
	// resize window to fit user's window
	DEVMODE devmode;
    devmode.dmPelsWidth = *screen_width;
    devmode.dmPelsHeight = *screen_height;
    devmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
    devmode.dmSize = sizeof(DEVMODE);

    long result = ChangeDisplaySettings(&devmode, 0);

	gint x = GetSystemMetrics(SM_CXSCREEN);
	gint y = GetSystemMetrics(SM_CYSCREEN);


	memcpy(screen_height,&y,sizeof(gint));
	memcpy(screen_width,&x,sizeof(gint));
}


void
qoe_setup(StreamConfig* qoe,
		  gint screen_width,
		  gint screen_height,
		  Codec audio_codec,
		  Codec video_codec,
		  QoEMode mode)
{
	
	qoe->screen_width =  screen_width;
	qoe->screen_height =  screen_height;

	display_setting_get_and_set(&qoe->screen_width,&qoe->screen_height);

	qoe->codec_audio = audio_codec;
	qoe->codec_video = video_codec;
	qoe->mode = mode;
}








Codec
qoe_get_audio_codec(StreamConfig* qoe)
{
	return qoe->codec_audio;
}

Codec
qoe_get_video_codec(StreamConfig* qoe)
{
	return qoe->codec_video;
}

gint 
qoe_get_screen_width(StreamConfig* qoe)
{
	return qoe->screen_width;
}

gint 
qoe_get_screen_height(StreamConfig* qoe)
{
	return qoe->screen_height;
}