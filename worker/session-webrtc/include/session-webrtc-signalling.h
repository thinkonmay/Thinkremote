                /// <summary>
/// @file session-webrtc-signalling.h
/// @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
/// </summary>
/// @version 1.0
/// @date 2021-09-05
/// 
/// @copyright Copyright (c) 2021
/// 
#ifndef __SESSION_CORE_SIGNALLING_H__
#define __SESSION_CORE_SIGNALLING_H__

#include "session-webrtc-type.h"
#include "session-webrtc.h"
#include "session-webrtc-type.h"


#include <message-form.h>

#include <libsoup/soup.h>
#include <glib.h>
#include <gst/gst.h>

/*Used as webrtcbin callback function*/

/**
 * @brief 
 * send ice candidate to webrtc peer
 * @param webrtc 
 * @param mlineindex 
 * @param candidate 
 * @param core 
 */
void                            send_ice_candidate_message                              (GstElement* webrtc,
                                                                                        guint mlineindex,
                                                                                        gchar* candidate,
                                                                                        SessionCore* core);

/**
 * @brief j
 * on negotiation needed signall emit by webrtc bin for signal the start of signalling procedure
 * @param element 
 * @param core 
 */
void                            on_negotiation_needed                                   (GstElement* element,
                                                                                        SessionCore* core);


/**
 * @brief 
 * connect session core pipeline with corresponding signalling handler function
 * @param core 
 */
void                            connect_signalling_handler                              (SessionCore* core);

/**
 * @brief 
 * on ice gathering state notify
 * @param webrtcbin 
 * @param pspec 
 * @param user_data 
 */
void                            on_ice_gathering_state_notify                           (GstElement* webrtcbin,
                                                                                        GParamSpec* pspec,
                                                                                        gpointer user_data);

/**
 * @brief 
 * initialize signalling hub from session core
 * @param core session core, parent of signalling hub
 * @return SignallingHub* pointer to signalling hub
 */
SignallingHub*                  signalling_hub_initialize                               (SessionCore* core);


/**
 * @brief 
 * start connecting with signalling server
 * @param core 
 */
void                            signalling_connect                                      (SessionCore* core);

/**
 * @brief 
 * setup signalling information 
 * @param hub signalling hub
 * @param turn turn connection, follow the turn connection string format of gstreamer and be prepared by host
 * @param url url of signalling server that session core establish connection with
 * @param stun_array stun array for ice gathering 
 * @param remote_token remote token used as a query parameter to connect with host
 */
void                            signalling_hub_setup                                    (SignallingHub* hub,
                                                                                        gchar* turn, 
                                                                                        gchar* url,
                                                                                        JsonArray* stun_array,
                                                                                        gchar* remote_token);


/**
 * @brief 
 * setup turn and stun server for webrtcbin for ice gathering process
 */
void                            signalling_hub_setup_turn_and_stun                      (Pipeline* pipeline,
                                                                                        SignallingHub* hub);


#endif // !__SESSION_CORE_SIGNALLING_H