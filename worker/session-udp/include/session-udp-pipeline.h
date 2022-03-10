/**
 * @file session-udp-pipeline.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2022-02-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <session-udp-type.h>
#include <gst/gst.h>

#include <session-udp-type.h>
#include <enum.h>




/**
 * @brief Set the up pipeline object
 * setup pipeline, 
 * this function wrap around element creating is property setup procedure
 * @param core 
 */
void                setup_pipeline					(SessionUdp* core,
                                                     JsonObject* object);


/**
 * @brief 
 * initialize pipeline object without setup the gstelement
 * @param core 
 * @return Pipeline* return to pipeline element
 */
Pipeline*			pipeline_initialize				();
