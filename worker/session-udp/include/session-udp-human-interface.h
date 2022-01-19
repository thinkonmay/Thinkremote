#include <session-udp-type.h>
#include "session-udp.h"

#include <glib.h>
#include "session-udp-type.h"
#include <gst/webrtc/webrtc_fwd.h>





/**
 * @brief 
 * initialize webrtchub with default member
 * @return HumanInterface* 
 */
HumanInterface*				human_interface_initialize						();


/**
 * @brief 
 * 
 */
void                        on_human_interface_message                  (gchar* message,
                                                                         SessionUdp* core);



