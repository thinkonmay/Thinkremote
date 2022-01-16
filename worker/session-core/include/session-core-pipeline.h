/// <summary>
/// @file session-core-pipeline.h
/// @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
/// </summary>
/// @version 1.0
/// @date 2021-09-05
/// 
/// @copyright Copyright (c) 2021
/// 
#include <session-core-type.h>
#include <gst/gst.h>

#include <session-core-type.h>
#include <qoe.h>




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
 * @brief 
 * set worker pointer to on or off
 * @param toggle 
 * @param core 
 */
void                toggle_pointer                  (gboolean toggle, 
                                                     SessionCore* core);