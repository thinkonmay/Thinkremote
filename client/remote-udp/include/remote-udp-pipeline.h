/**
 * @file remote-udp-pipeline.h
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
#include <remote-udp-type.h>
#include <gst/gst.h>

#include <remote-udp-type.h>
#include <enum.h>




/**
 * @brief Set the up pipeline object
 * prepare and setup pipeline by creating gstelement and connect necessary signal handler
 * @param core 
 * @return gpointer 
 */
gpointer			setup_pipeline					(RemoteUdp* core);



/**
 * @brief 
 * initialize pipeline
 * @param core 
 * @return Pipeline* newly created pipeline
 */
Pipeline*			pipeline_initialize				();


/**
 * @brief Set the up pipeline startpoint object
 * 
 * @param pipeline 
 * @param audio_port 
 * @param video_port 
 */
void                setup_pipeline_startpoint       (Pipeline* pipeline,
                                                    gint audio_port,
                                                    gint video_port);


#endif