/**
 * @file remote-udp.h
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

#include <remote-udp-type.h>
#include <message-form.h>
#include <overlay-gui.h>






	

/**
 * @brief 
 * initialize remote session
 * @param remote_token 
 * @return RemoteUdp* 
 */
RemoteUdp*		remote_app_initialize				(gchar* remote_token);

/**
 * @brief 
 * reset remote-udp-stream
 * @param self 
 */
void 			remote_app_reset					(RemoteUdp* self);

/**
 * @brief 
 * finalize remote app and end the remote connection
 * @param self 
 * @param exit_code 
 * @param error 
 */
void			remote_app_finalize					(RemoteUdp* self,
													GError* error);

/**
 * @brief 
 * get pipeline from remote app
 * @param self 
 * @return Pipeline* 
 */
Pipeline*		remote_app_get_pipeline				(RemoteUdp* self);


/**
 * @brief 
 * get qoe object from remtoe app
 * @param self 
 * @return RemoteConfig* 
 */
RemoteConfig*			remote_app_get_qoe			(RemoteUdp* self);



/**
 * @brief 
 * remote report app error to host
 * @param self 
 * @param error message
 */
void			report_remote_app_error				(RemoteUdp* self,
													gchar* error);

/**
 * @brief 
 * get handler handler from remote app
 * @param app 
 * @return HIDHandler* 
 */
InputHandler* 	remote_app_get_hid_handler			(RemoteUdp* app);


/**
 * @brief 
 * get gui from remote app
 * @param core 
 * @return GUI* 
 */
GUI* 			remote_app_get_gui					(RemoteUdp* core);
#endif 