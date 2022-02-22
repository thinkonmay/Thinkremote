/**
 * @file qoe.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2022-02-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */
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
}QoEMode;


#endif