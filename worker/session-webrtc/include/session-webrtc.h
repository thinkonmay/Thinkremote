/**
 * @file session-webrtc.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2022-02-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __SESSION_CORE_H__
#define __SESSION_CORE_H__

#include <session-webrtc-type.h>
#include <json-handler.h>
#include <enum.h>






	

/**
 * @brief 
 * initialize session core, 
 * initialization of session core do not need any input parameter, 
 * instead, session will automatically their required parameter from global-var.h
 * all operation needed to establishing a remote connection is already called inside session core initialize
 * @return SessionCore* pointer to session core instance
 */
SessionCore*	session_core_initialize					();



/**
 * @brief 
 * end session core and send error code to cluster manager
 * session core finalize simply terminate the window and exit the program
 * @param self 
 * @param error 
 */
void			session_core_finalize					(SessionCore* self,
														GError* error);

/**
 * @brief 
 * get pipeline from session core
 * @param self session core
 * @return Pipeline* pointer to pipeline
 */
Pipeline*		session_core_get_pipeline				(SessionCore* self);

/**
 * @brief 
 * get webrtc hub from session core
 * @param self session core
 * @return WebRTCHub*  pointer to webrtc
 */
WebRTCHub*		session_core_get_rtc_hub				(SessionCore* self);

/**
 * @brief 
 * get qoe from session core
 * @param self session core
 * @return StreamConfig* qoe
 */
StreamConfig*			session_core_get_qoe					(SessionCore* self);



/**
 * @brief 
 * get signalling hub from session core
 * @param self session core
 * @return SignallingHub* 
 */
SignallingHub*	session_core_get_signalling_hub			(SessionCore* self);


/**
 * @brief 
 * get remote token from session core
 * @param self session core
 * @return gchar* remote token get from cluster
 */
gchar*			session_core_get_remote_token			(SessionCore* self);
#ifndef G_OS_WIN32
#include <Xlib.h>

Display* 		session_core_display_interface			(SessionCore* self);
#endif

#endif 