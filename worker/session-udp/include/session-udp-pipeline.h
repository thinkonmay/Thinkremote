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
void                setup_pipeline					(SessionUdp* core);


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
                                                     SessionUdp* core);