/**
 * @file remote-webrtc-signalling.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __REMOTE_APP_SIGNALLING_H__
#define __REMOTE_APP_SIGNALLING_H__

#include "remote-webrtc-type.h"
#include "remote-webrtc.h"
#include "remote-webrtc-type.h"


#include <message-form.h>

#include <libsoup/soup.h>
#include <glib.h>
#include <gst/gst.h>


/**
 * @brief 
 * intialize signalling hub by allocate dynamic memory
 * @param core 
 * @return SignallingHub* 
 */
SignallingHub*                  signalling_hub_initialize                               (RemoteApp* core);

/**
 * @brief 
 * connect with signalling server using remote token
 * @param core 
 */
void                            signalling_connect                                      (RemoteApp* core);


/**
 * @brief 
 * register with server
 * @param core 
 * @return gboolean 
 */
gboolean                        register_with_server                                    (RemoteApp* core);

/**
 * @brief 
 * setup signalling hub with necessary url
 * @param hub signalling hub
 * @param turn turn connection
 * @param url 
 * @param stun_array
 * @param remote_token
 */
void                            signalling_hub_setup                                    (SignallingHub* hub, 
                                                                                        gchar* turn,
                                                                                        gchar* url,
                                                                                        JsonArray* stun_array,
                                                                                        gchar* remote_token);

/**
 * @brief 
 * close signallling message,
 * this will also end the stream
 * @param hub 
 * @return gboolean 
 */
gboolean                        signalling_close                                        (SignallingHub* hub);

/**
 * @brief 
 * connect webrtcbin with necessary signalling handler
 * @param core 
 */
void                            connect_signalling_handler                              (RemoteApp* core);



#endif 