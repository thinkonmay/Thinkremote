/// <summary>
/// @file session-udp.h
/// @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
/// </summary>
/// @version 1.0
/// @date 2021-09-05
/// 
/// @copyright Copyright (c) 2021
/// 
#ifndef __SESSION_CORE_H__
#define __SESSION_CORE_H__

#include <session-udp-type.h>
#include <message-form.h>
#include <human-interface-opcode.h>






	

/**
 * @brief 
 * initialize session core, 
 * initialization of session core do not need any input parameter, 
 * instead, session will automatically their required parameter from global-var.h
 * all operation needed to establishing a remote connection is already called inside session core initialize
 * @return SessionUdp* pointer to session core instance
 */
SessionUdp*	session_core_initialize					();



/**
 * @brief 
 * end session core and send error code to cluster manager
 * session core finalize simply terminate the window and exit the program
 * @param self 
 * @param error 
 */
void			session_core_finalize					(SessionUdp* self,
														GError* error);

/**
 * @brief 
 * get pipeline from session core
 * @param self session core
 * @return Pipeline* pointer to pipeline
 */
Pipeline*		session_core_get_pipeline				(SessionUdp* self);

/**
 * @brief 
 * get webrtc hub from session core
 * @param self session core
 * @return HumanInterface*  pointer to webrtc
 */
HumanInterface*	session_core_get_rtc_hub			(SessionUdp* self);

/**
 * @brief 
 * get qoe from session core
 * @param self session core
 * @return StreamConfig* qoe
 */
StreamConfig*	session_core_get_qoe					(SessionUdp* self);



/**
 * @brief 
 * get signalling hub from session core
 * @param self session core
 * @return SignallingHub* 
 */
SignallingHub*	session_core_get_signalling_hub			(SessionUdp* self);


/**
 * @brief 
 * get remote token from session core
 * @param self session core
 * @return gchar* remote token get from cluster
 */
gchar*			session_core_get_remote_token			(SessionUdp* self);




/**
 * @brief 
 * get client device of current stream
 * @param app 
 * @return DeviceType 
 */
DeviceType		session_core_get_client_device			(SessionUdp* self);

/**
 * @brief 
 * 
 * @param self 
 * @return CoreEngine 
 */
CoreEngine 		session_core_get_client_engine			(SessionUdp* self);

#ifndef G_OS_WIN32
#include <Xlib.h>

Display* 		session_core_display_interface			(SessionUdp* self);
#endif

#endif 