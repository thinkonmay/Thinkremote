#include <remote-webrtc-type.h>
#include "remote-webrtc.h"

#include <glib.h>
#include "remote-webrtc-type.h"
#include <gst/webrtc/webrtc_fwd.h>

/**
 * @brief 
 * connect datachannel signals to corresponding signal handler
 * @param core 
 * @return gboolean 
 */
gboolean                connect_data_channel_signals                (RemoteApp* core);

/**
 * @brief 
 * initialize webrtchub by allocate memory
 * @return WebRTCHub* 
 */
WebRTCHub*				webrtchub_initialize						();



/**
 * @brief 
 * send message over hid data channel
 * @param message message string
 * @param core 
 */
void                    hid_data_channel_send                       (gchar* message,
                                                                     RemoteApp* core);


/**
 * @brief 
 * 
 * @param hub 
 */
void                    stop_to_ping                                (WebRTCHub* hub);