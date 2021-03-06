/// <summary>
/// @file session-udp-type.h
/// @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
/// </summary>
/// @version 1.0
/// @date 2021-09-05
/// 
/// 
/// @copyright Copyright (c) 2021
#pragma once
#ifndef  __SESSION_CORE_TYPE_H__
#define __SESSION_CORE_TYPE_H__

#include <json-glib-1.0/json-glib/json-glib.h>
#include <glib-2.0/glib.h>





/**
 * @brief 
 * Pipeline is a data structure that wrap around video and audio
 * handling functionality of session core  
 */
typedef struct 			_Pipeline 				                Pipeline;


/**
 * @brief 
 * SessionUdp is a data structure that wrap outside the session core module
 */
typedef struct 			_SessionUdp 			                SessionUdp;






#endif // ! __SESSION_CORE_TYPE_H__



