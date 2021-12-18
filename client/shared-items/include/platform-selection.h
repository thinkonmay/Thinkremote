#ifndef __PLATFORM_SELECTION_H__
#define __PLATFORM_SELECTION_H__

#include <glib.h>


#ifdef G_OS_WIN32

#define DEFAULT_VIDEO_SINK          "d3d11videosink"

#else

#endif


#endif