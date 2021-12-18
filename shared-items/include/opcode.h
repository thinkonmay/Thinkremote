/// <summary>
/// @file opcode.h
/// @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
/// </summary>
/// @version 1.0
/// @date 2021-09-06
/// 
/// 
/// @copyright Copyright (c) 2021
#ifndef __OPCODE_H__
#define __OPCODE_H__

typedef enum
{
    SESSION_INFORMATION,

    REGISTER_SLAVE,

    SLAVE_ACCEPTED,
    DENY_SLAVE,

    REJECT_SLAVE,

    SESSION_INITIALIZE,
    SESSION_TERMINATE,
    RECONNECT_REMOTE_CONTROL,
    DISCONNECT_REMOTE_CONTROL,

    RESET_QOE,

    SESSION_CORE_EXIT,
    ERROR_REPORT,

    NEW_SHELL_SESSION,
    END_SHELL_SESSION,

    FILE_TRANSFER_SERVICE,
    CLIPBOARD_SERVICE
}Opcode;


typedef enum
{
    FILE_METADATA,
    FILE_TRANSFER_OK,
    END_OF_FILE
}FileTransferOpcode;

#endif