/**
 * @file agent-child-process.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __AGENT_CHILD_PROCESS_H__
#define __AGENT_CHILD_PROCESS_H__
#include "agent-type.h"
#include "glib.h"


/**
 * @brief 
 * send message to child process over standard input and output
 * @param self child process
 * @param buffer buffer send to child process
 * @param size size of buffer
 * @return gboolean 
 */
gboolean			send_message_to_child_process				(ChildProcess* self,
																 gchar* buffer,
																 gint size);

/// <summary>
/// close child process
/// </summary>
/// <param name="proc">child process which will be close</param>
void				wait_for_childproces						(ChildProcess* process);



/**
 * @brief Create a new child process object
 * create child process with handler function
 * @param binary_name name of child process
 * @param stderrhdl standard err handler 
 * @param stdouthdl standard output handler
 * @param handler child process termination handling
 * @param agent agent module
 * @param data 
 * @return ChildProcess* 
 */
ChildProcess*		create_new_child_process					(gchar* binary_name,
																 ChildStdErrHandle stderrhdl,
																 ChildStdOutHandle stdouthdl,
																 ChildStateHandle handler,
																 AgentServer* agent,
																 gpointer data);


/**
 * @brief 
 * force child process to terminate
 * @param proc 
 */
void 				childprocess_force_exit						(ChildProcess* proc);

/**
 * @brief 
 * clean child process memory segment 
 * @param proc 
 */
void 				clean_childprocess							(ChildProcess* proc);
#endif