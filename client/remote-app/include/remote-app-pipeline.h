/**
 * @file remote-app-pipeline.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __REMOTE_APP_PIPELINE_H__
#define __REMOTE_APP_PIPELINE_H__
#include <remote-app-type.h>
#include <gst/gst.h>

#include <remote-app-type.h>
#include <qoe.h>




/**
 * @brief Set the up pipeline object
 * prepare and setup pipeline by creating gstelement and connect necessary signal handler
 * @param core 
 * @return gpointer 
 */
gpointer			setup_pipeline					(RemoteApp* core);




/**
 * @brief 
 * get webrtcbin from pipeline
 * @param pipeline 
 * @return GstElement* webrtcbin
 */
GstElement*			pipeline_get_webrtc_bin			(Pipeline* pipeline);

/**
 * @brief 
 * initialize pipeline
 * @param core 
 * @return Pipeline* newly created pipeline
 */
Pipeline*			pipeline_initialize				(RemoteApp* core);


/**
 * @brief 
 * 
 * @param pipeline 
 * @return GstElement* 
 */
GstElement*         pipeline_get_pipeline_element   (Pipeline* pipeline);
#endif