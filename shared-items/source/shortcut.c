/**
 * @file shortcut.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2022-02-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <shortcut.h>



Shortcut*               
shortcut_list_initialize(gint size)
{
    Shortcut* shortcuts = malloc(sizeof(Shortcut)*size);
    memset(shortcuts,0,sizeof(Shortcut)*size);
    return shortcuts;
}


void                    
shortcut_list_free(Shortcut* shortcuts)
{
    free(shortcuts);
}

void               
add_new_shortcut_to_list(Shortcut* shortcuts,
                    gint* keys,
                    ShortcutOpcode opcode,
                    ShortcutHandleFunction function,
                    gpointer data)
{
    gint i = 0;
    while ((shortcuts + i)->active) { i++; }

    (shortcuts + i)->data = data;
    (shortcuts + i)->function = function;
    (shortcuts + i)->opcode = opcode;
    (shortcuts + i)->active = TRUE;

    if(!keys)
        return;

    gint k = 0;
    while (*(keys + k)) 
    {
        (shortcuts + i)->key_list[k] = *(keys + k);
        k++;
    }
}