/**
 * @file global-var.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <global-var.h>



void
default_var()
{
    AGENT_PORT = malloc(10);
    SESSION_CORE_PORT = malloc(10);
    CLUSTER_URL = malloc(200);

    TOKEN = malloc(500);
    DEVICE_TOKEN = malloc(500);
    USER = malloc(100);
    PASSWORD = malloc(100);

    memset(AGENT_PORT,0,10);
    memset(SESSION_CORE_PORT,0,10);
    memset(CLUSTER_URL,0,200);

    memset(TOKEN,0,500);
    memset(DEVICE_TOKEN,0,500);
    memset(USER,0,100);
    memset(PASSWORD,0,100);

    SESSION_CORE_PORT = "3330";
    AGENT_PORT = "2220";
}
