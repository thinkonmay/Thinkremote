
#include <glib-2.0/glib.h>

#ifdef G_OS_WIN32
#include <Windows.h>

gboolean
SetPermanentEnvironmentVariable(LPCTSTR value, 
                                LPCTSTR data)
{
    HKEY hKey;
    LPCTSTR keyPath = TEXT("Environment");
    LSTATUS lOpenStatus = RegOpenKeyEx(HKEY_CURRENT_USER, keyPath, 0, KEY_ALL_ACCESS, &hKey);
    if (lOpenStatus == ERROR_SUCCESS) 
    {
        LSTATUS lSetStatus = RegSetValueEx(hKey, value, 0, REG_SZ,(LPBYTE)data, strlen(data) + 1);
        RegCloseKey(hKey);

        if (lSetStatus == ERROR_SUCCESS)
        {
            SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_BLOCK, 100, NULL);
            return TRUE;
        }
    }

    return FALSE;
}

gchar*
GetEnvironmentVariableWithKey(gchar* key)
{
    gchar* value = malloc(100);
    memset(value,0,100);

    LPTCH lpvEnv; 
    LPTSTR lpszVariable; 
 
    lpvEnv = GetEnvironmentStrings();
    lpszVariable = (LPTSTR) lpvEnv;
    while (*lpszVariable)
    {
        GString* string = g_string_new(key);
        g_string_append(string,"=");
        gchar* desire_prefix = g_string_free(string,FALSE); 


        gchar* temp = lpszVariable;
        if(g_str_has_prefix(temp,desire_prefix))
        {
            gchar* after = temp + strlen(desire_prefix);
            memcpy(value,after,strlen(after));
            return value;
        }
        lpszVariable += lstrlen(lpszVariable) + 1;
    }
    FreeEnvironmentStrings(lpvEnv);
    if(!strlen(value))
    {
        free(value);
        return NULL;
    }
}
#else




#endif