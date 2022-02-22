
#include <glib-2.0/glib.h>
#ifdef G_OS_WIN32
#include <Windows.h>

/**
 * @brief Set the Permanent Environment Variable object
 * 
 * @param value 
 * @param data 
 * @return gboolean 
 */
gboolean    SetPermanentEnvironmentVariable     (LPCTSTR value, 
                                                 LPCTSTR data);

/**
 * @brief 
 * 
 */
gchar*      GetEnvironmentVariableWithKey       (gchar* key);
#endif

