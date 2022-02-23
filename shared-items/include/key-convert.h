/**
 * @file key-convert.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __KEY_CONVERT_H__
#define __KEY_CONVERT_H__
#include <glib.h>


/**
 * @brief 
 * convert key from javascript standard to window 
 * @param keycode 
 * @return unsigned short 
 */
unsigned short          convert_javascript_key_to_window_key            (gchar* keycode);


/**
 * @brief 
 * 
 */
typedef enum _WindowKeyCode
{
    KEY_0 = 0x30,	
    KEY_1 = 0x31,	
    KEY_2 = 0x32,	
    KEY_3 = 0x33,	
    KEY_4 = 0x34,	
    KEY_5 = 0x35,	
    KEY_6 = 0x36,	
    KEY_7 = 0x37,	
    KEY_8 = 0x38,	
    KEY_9 = 0x39,	
    A_KEY = 0x41,	
    B_KEY = 0x42,	
    C_KEY = 0x43,	
    D_KEY = 0x44,	
    E_KEY = 0x45,	
    F_KEY = 0x46,	
    G_KEY = 0x47,	
    H_KEY = 0x48,	
    I_KEY = 0x49,	
    J_KEY = 0x4A,	
    K_KEY = 0x4B,	
    L_KEY = 0x4C,	
    M_KEY = 0x4D,	
    N_KEY = 0x4E,	
    O_KEY = 0x4F,	
    P_KEY = 0x50,	
    Q_KEY = 0x51,	
    R_KEY = 0x52,	
    S_KEY = 0x53,	
    T_KEY = 0x54,	
    U_KEY = 0x55,	
    V_KEY = 0x56,	
    W_KEY = 0x57,	
    X_KEY = 0x58,	
    Y_KEY = 0x59,	
    Z_KEY = 0x5A,	
}WindowKeyCode;



#endif