/**
 * @file remote-webrtc-type.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __REMOTE_APP_TYPE_H__
#define __REMOTE_APP_TYPE_H__
#include <glib-2.0/glib.h>






/**
 * @brief 
 * pipeline is a datastructure that wrap around handling of gstelement
 */
typedef struct 			_Pipeline 				            Pipeline;

/**
 * @brief 
 * remote app is a data structure that wrap around all functionality of remote app
 */
typedef struct 			_RemoteApp 			                RemoteApp;

/**
 * @brief 
 * WebRTCHub is a datastructure that wrap around communication method with session core
 */
typedef struct			_WebRTCHub				                WebRTCHub;

/**
 * @brief 
 * Signalling hub is a datastructure that wrap around connection of remote app with signalling hub
 */
typedef struct 			_SignallingHub			                SignallingHub;




#endif

