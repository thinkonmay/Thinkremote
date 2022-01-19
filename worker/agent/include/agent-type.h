/**
 * @file agent-type.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#ifndef __AGENT_TYPE_H__
#define __AGENT_TYPE_H__

#include <glib.h>
#include <json-glib/json-glib.h>




/**
 * @brief 
 * Socket is a object that wrap around agent connection with cluster manager
 */
typedef struct _Socket 				Socket;

/**
 * @brief 
 * Agent server is a data structure that wrap around agent module
 */
typedef struct _AgentServer			AgentServer;

/**
 * @brief 
 * ChildProcess is a data structure wrap around child process creation and termination
 */
typedef	struct _ChildProcess		ChildProcess;

/**
 * @brief 
 * 
 */
typedef struct _PortForward         PortForward;

/**
 * @brief 
 * Shellsession is a datastructure that wrap around shell session execution
 */
typedef struct _ShellSession        ShellSession;

/**
 * @brief 
 * Remote session is a data structure that wrap around remote service
 */
typedef struct _RemoteSession       RemoteSession;





/**
 * @brief 
 * Child stdout handle is a function that will be 
 * callback for every new output buffer from child process
 */
typedef void        (*ChildStdOutHandle)    (GBytes* buffer,
                                            AgentServer* agent,
                                            gpointer data);

/**
 * @brief 
 * Child stdout handle is a function that will be 
 * callback for every new stderr buffer from child process
 */
typedef void        (*ChildStdErrHandle)    (GBytes* buffer,
                                            AgentServer* agent,
                                            gpointer data);
/**
 * @brief 
 * ChildStateHandle is a function that handle self termination of child process
 */
typedef void        (*ChildStateHandle)     (ChildProcess* proc,
                                            AgentServer* agent,
                                            gpointer data);


#endif