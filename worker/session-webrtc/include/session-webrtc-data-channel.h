#include <session-webrtc-type.h>
#include "session-webrtc.h"

#include <glib.h>
#include "session-webrtc-type.h"
#include <gst/webrtc/webrtc_fwd.h>




/**
 * @brief 
 * connect datachannel signal with corresponding message handler
 * @param core 
 * @return gboolean 
 */
gboolean                connect_data_channel_signals                (SessionCore* core);



/**
 * @brief 
 * initialize webrtchub with default member
 * @return WebRTCHub* 
 */
WebRTCHub*				webrtchub_initialize						();





