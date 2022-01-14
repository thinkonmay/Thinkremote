/**
 * @file agent-libsoup-server.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2022-01-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __AGENT_LIBSOUP_SERVER_H__
#define __AGENT_LIBSOUP_SERVER_H__
#include <libsoup/soup.h>
#include <agent-type.h>

SoupServer*         init_agent_server           (AgentServer* agent,
                                                gboolean self_host);
#endif