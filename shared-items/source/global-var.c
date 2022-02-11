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
thinkremote_application_init()
{
    CLUSTER_URL = malloc(200);
    DEVICE_TOKEN = malloc(500);
    CLUSTER_TOKEN = malloc(500);

    memset(CLUSTER_URL,0,200);
    memset(DEVICE_TOKEN,0,500);
    memset(CLUSTER_TOKEN,0,500);
}
