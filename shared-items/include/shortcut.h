/**
 * @file shortcut.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2022-02-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __SHORTCUT_H__
#define __SHORTCUT_H__
#include <glib-2.0/glib.h>
#include <enum.h>


/**
 * @brief 
 * 
 */
typedef void            (*ShortcutHandleFunction)                (gpointer data);

/**
 * @brief 
 * 
 */
typedef struct _Shortcut
{
    ShortcutOpcode opcode;

    gint key_list[10];

    gpointer data;

    ShortcutHandleFunction function;

    gboolean active;
}Shortcut;


/**
 * @brief 
 * 
 * @param size 
 * @return Shortcut* 
 */
Shortcut*               shortcut_list_initialize        (gint size);


/**
 * @brief 
 * 
 * @param shortcuts 
 */
void                    shortcut_list_free              (Shortcut* shortcuts);


/**
 * @brief 
 * 
 * @param shortcuts 
 * @param keys 
 * @param opcode 
 * @param function 
 * @param data 
 */
void                    add_new_shortcut_to_list        (Shortcut* shortcuts,
                                                        gint* keys,
                                                        ShortcutOpcode opcode,
                                                        ShortcutHandleFunction function,
                                                        gpointer data);



#endif