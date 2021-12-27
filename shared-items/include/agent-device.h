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

#ifdef G_OS_WIN32
#include <Windows.h>	
#include <stdio.h>
#include <sysinfoapi.h>
#include <d3d9.h>
#include <intrin.h>
#include <glib.h>



#pragma comment(lib, "d3d9.lib")





/**
 * @brief Get the registration message object
 * this function wrap around process of getting worker information
 * @return gchar* registration message string 
 */
gchar*                  get_registration_message        ();



/**
 * @brief Get the local ip object
 * 
 * @return gchar* 
 */
gchar*                  get_local_ip                    ();

#endif
#endif
