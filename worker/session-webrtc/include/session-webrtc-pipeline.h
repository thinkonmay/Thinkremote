/**
 * @file session-webrtc-pipeline.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2022-02-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __SESSION_WEBRTC_PIPELINE_H__
#define __SESSION_WEBRTC_PIPELINE_H__

#include <session-webrtc-type.h>

#include <gst/gst.h>

#include <enum.h>




/**
 * @brief Set the up pipeline object
 * setup pipeline, 
 * this function wrap around element creating is property setup procedure
 * @param core 
 */
void                setup_pipeline					(SessionCore* core);

/**
 * @brief 
 * get webrtcbin element from pipeline, 
 * webrtcbin is a special function used for handling webrtcb connection 
 * @param pipeline 
 * @return GstElement* 
 */
GstElement*			pipeline_get_webrtc_bin			(Pipeline* pipeline);

/**
 * @brief 
 * initialize pipeline object without setup the gstelement
 * @param core 
 * @return Pipeline* return to pipeline element
 */
Pipeline*			pipeline_initialize				();

/**
 * @brief Set the up media device and stream object
 * 
 * @param pipe 
 * @param object 
 */
void                setup_media_device_and_stream   (Pipeline* pipe,
                                                     JsonObject* object);
#endif