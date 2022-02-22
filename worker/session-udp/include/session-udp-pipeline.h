/// <summary>
/// @file session-udp-pipeline.h
/// @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
/// </summary>
/// @version 1.0
/// @date 2021-09-05
/// 
/// @copyright Copyright (c) 2021
/// 
#include <session-udp-type.h>
#include <gst/gst.h>

#include <session-udp-type.h>
#include <qoe.h>




/**
 * @brief Set the up pipeline object
 * setup pipeline, 
 * this function wrap around element creating is property setup procedure
 * @param core 
 */
void                setup_pipeline					(SessionUdp* core,
                                                     UdpEndpoint endpoint);


/**
 * @brief 
 * initialize pipeline object without setup the gstelement
 * @param core 
 * @return Pipeline* return to pipeline element
 */
Pipeline*			pipeline_initialize				();

/**
 * @brief 
 * 
 * @param pipeline 
 * @return GstElement* 
 */
GstElement*         pipeline_get_screen_capture_element (Pipeline* pipeline);