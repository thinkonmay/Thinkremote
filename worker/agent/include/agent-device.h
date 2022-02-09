/**
 * @file agent-device.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#ifndef __AGENT_DEVICE_H__
#define __AGENT_DEVICE_H__



#include <glib.h>







/**
 * @brief Get the registration message object
 * this function wrap around process of getting worker information
 * 
 * @param port_forward 
 * @param agent_instance_port 
 * @return gchar* registration message string 
 */
gchar*                  get_registration_message        (gboolean port_forward, 
						                                 gchar* agent_instance_port);
#endif
