/// <summary>
/// @file qoe.h
/// @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
/// </summary>
/// @version 1.0
/// @date 2021-09-06
/// 
/// 
/// @copyright Copyright (c) 2021
#ifndef __QOE_H__
#define __QOE_H__

typedef enum
{
    CODEC_H265,
    CODEC_H264,
    CODEC_VP8,
    CODEC_VP9,

    OPUS_ENC,
    AAC_ENC
}Codec;


typedef enum
{
    ULTRA_LOW_CONST = 1,
    LOW_CONST,
    MEDIUM_CONST,
    HIGH_CONST,
    VERY_HIGH_CONST,
    ULTRA_HIGH_CONST,

    SEGMENTED_ADAPTIVE_BITRATE,
    NON_OVER_SAMPLING_ADAPTIVE_BITRATE,
    OVER_SAMPLING_ADAPTIVE_BITRATE,
    PREDICTIVE_ADAPTIVE_BITRATE
}QoEMode;




#endif