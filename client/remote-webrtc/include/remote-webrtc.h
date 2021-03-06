/**
 * @file remote-webrtc.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __REMOTE_APP_H__
#define __REMOTE_APP_H__

#include <remote-webrtc-type.h>
#include <json-handler.h>
#ifdef G_OS_WIN32
#include <overlay-gui.h>
#endif





	

/**
 * @brief 
 * initialize remote session
 * @param remote_token 
 * @return RemoteApp* 
 */
RemoteApp*		remote_app_initialize				(gchar* remote_token);

/**
 * @brief 
 * reset remote-webrtc-stream
 * @param self 
 */
void 			remote_app_reset					(RemoteApp* self);

/**
 * @brief 
 * finalize remote app and end the remote connection
 * @param self 
 * @param exit_code 
 * @param error 
 */
void			remote_app_finalize					(RemoteApp* self,
														GError* error);

/**
 * @brief 
 * get pipeline from remote app
 * @param self 
 * @return Pipeline* 
 */
Pipeline*		remote_app_get_pipeline				(RemoteApp* self);

/**
 * @brief 
 * get webrtc hub from remote app
 * @param self 
 * @return WebRTCHub* 
 */
WebRTCHub*		remote_app_get_rtc_hub				(RemoteApp* self);



/**
 * @brief 
 * get signalling hub from remote app
 * @param core 
 * @return SignallingHub* signalling hub
 */
SignallingHub*	remote_app_get_signalling_hub			(RemoteApp* core);


/**
 * @brief 
 * remote report app error to host
 * @param self 
 * @param error message
 */
void			report_remote_app_error				(RemoteApp* self,
													gchar* error);



#ifdef G_OS_WIN32
/**
 * @brief 
 * get gui from remote app
 * @param core 
 * @return GUI* 
 */
GUI* 			remote_app_get_gui					(RemoteApp* core);
#endif
#endif 