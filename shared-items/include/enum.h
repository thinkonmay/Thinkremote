/**
 * @file enum.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __HUMAN_INTERFACE_OPCODE_H__
#define __HUMAN_INTERFACE_OPCODE_H__


/**
 * @brief 
 * HID Opcode for communication with client 
 */
typedef enum
{
	KEYUP,
	KEYDOWN,

	MOUSE_WHEEL,
	MOUSE_MOVE,
	MOUSE_UP,
	MOUSE_DOWN,
}JavaScriptOpcode;

typedef enum
{
	POINTER_LOCK = 100,
	RESET_KEY,
	FULLSCREEN,
	RELOAD_STREAM,
}ShortcutOpcode;

typedef enum
{
	KEYRAW = 200,
	MOUSERAW,

	MOUSEWHEEL,

	GAMEPAD_IN,
	GAMEPAD_OUT,
}Win32Opcode;


typedef enum 
{
	WEB_APP,
	WINDOW_APP,
	LINUX_APP,
	MAC_OS_APP,
	ANDROID_APP,
	IOS_APP
}DeviceType;

/**
 * @brief 
 * 
 */
typedef enum 
{
	GSTREAMER,
	CHROME
}CoreEngine;

/**
 * @brief 
 * 
 */
typedef enum
{
    CODEC_H265,
    CODEC_H264,
    CODEC_VP8,
    CODEC_VP9,

    OPUS_ENC,
    AAC_ENC
}Codec;

/**
 * @brief 
 * 
 */
typedef enum
{
    ULTRA_LOW_CONST = 1,
    LOW_CONST,
    MEDIUM_CONST,
    HIGH_CONST,
    VERY_HIGH_CONST,
    ULTRA_HIGH_CONST,
}QoEMode;


#endif