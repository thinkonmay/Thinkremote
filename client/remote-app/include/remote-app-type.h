/**
 * @file remote-app-type.h
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
 * hid handler is a datastructure that wrap around handling of 
 */
typedef struct          _InputHandler                         InputHandler;

/**
 * @brief 
 * GUI is a datastructure wrap around creation of remote window and win32 input handling
 */
typedef struct 			_GUI 				                GUI;

/**
 * @brief 
 * remote app is a data structure that wrap around all functionality of remote app
 */
typedef struct 			_RemoteApp 			                RemoteApp;

/**
 * @brief 
 * QoE is a datastructure that wrap around collection and management of streaming quality
 */
typedef struct 			_QoE					                QoE;

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

/**
 * @brief 
 * Hid Input is a datastructure that wrap around all parameter 
 * that captured by remoteapp related to human interface device
 */
typedef struct			_HidInput								HidInput;



#endif

