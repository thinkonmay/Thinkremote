/**
 * @file session-udp-remote-config.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <session-udp-remote-config.h>
#include <session-udp-type.h>
#include <session-udp-pipeline.h>

#include <enum.h>

#include <gst/gst.h>
#include <glib-2.0/glib.h>


#include <stdio.h>




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


#ifdef G_OS_WIN32
#include <Windows.h>
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
#endif


void
qoe_setup(StreamConfig* qoe,
		  gint screen_width,
		  gint screen_height,
		  Codec audio_codec,
		  Codec video_codec,
		  QoEMode mode)
{
#ifdef G_OS_WIN32
	display_setting_get_and_set(&qoe->screen_width,&qoe->screen_height);
#endif
	
	qoe->screen_width =  screen_width;
	qoe->screen_height =  screen_height;

	qoe->codec_audio = audio_codec;
	qoe->codec_video = video_codec;

	qoe->mode = mode;
}





gint 
qoe_get_video_bitrate(StreamConfig* qoe)
{
	switch (qoe->mode)
	{
	case ULTRA_LOW_CONST:
		return 2000;
	case LOW_CONST:
		return 3000;
	case MEDIUM_CONST:
		return 4000;
	case HIGH_CONST:
		return 5000;
	case VERY_HIGH_CONST:
		return 6000;
	case ULTRA_HIGH_CONST:
		return 7000;
	default:
		return 5000;
	}
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