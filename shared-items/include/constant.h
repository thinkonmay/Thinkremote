/**
 * @file constant.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __DEVELOPMENT_H__
#define __DEVELOPMENT_H__
#define GST_USE_UNSTABLE_API

#include <glib.h>
#include <environment.h>




/**
 * @brief 
 * 
 */
// #define DEFAULT_TURN                       "turn://359549596:2000860796@13.214.177.108:3478"

/**
 * @brief 
 * 
 */
#define DEFAULT_STUN                       "stun.l.google.com:19302"

/**
 * @brief 
 * 
 */
#define THINKMAY_DOMAIN                     "host.thinkmay.net" 

/**
 * @brief 
 * 
 */
#define LOG_URL                             "https://"THINKMAY_DOMAIN"/Log/Worker"

/**
 * @brief 
 * 
 */
#define ACCOUNT_URL                         "https://"THINKMAY_DOMAIN"/Account/Login"

/**
 * @brief 
 * 
 */
#define CLUSTER_TOKEN_URL                   "https://"THINKMAY_DOMAIN"/Cluster/Token"

/**
 * @brief 
 * 
 */
#define CLUSTER_INFOR_URL                   "https://"THINKMAY_DOMAIN"/Cluster/Infor"

/**
 * @brief 
 * 
 */
#define INSTANCE_INFOR_URL                  "https://"THINKMAY_DOMAIN"/Cluster/Instance"

/**
 * @brief 
 * 
 */
#define SESSION_URL                         "https://"THINKMAY_DOMAIN"/Session/Setting"

/**
 * @brief 
 * 
 */
#define PORT_OBTAIN_URL                      "https://"THINKMAY_DOMAIN"/Port/Request"

/**
 * @brief 
 * 
 */
#define PORT_RELEASE_URL                     "https://"THINKMAY_DOMAIN"/Port/Release?InstancePort="

/**
 * @brief 
 * 
 */
#define DEFAULT_CLIENT_TOKEN                 "TestingClientModuleToken"

/**
 * @brief 
 * 
 */
#define DEFAULT_CORE_TOKEN                   "TestingCoreModuleToken"


/**
 * @brief 
 * 
 */
#define DEVELOPMENT_SIGNALLING_URL           GetEnvironmentVariableWithKey("SIGNALLING")


/**
 * @brief 
 * 
 */
#define SESSION_CORE_BINARY                 "session-webrtc.exe"

/**
 * @brief 
 * 
 */
#define AGENT_BINARY                        "agent.exe"

/**
 * @brief 
 * 
 */
#define REMOTE_APP_BINARY                   "remote-webrtc.exe"

/**
 * @brief 
 * 
 */
#define PORT_FORWARD_BINARY                 "port-forward.exe"


/**
 * @brief 
 * 
 */
#define OFFER_ICE                           "OFFER_ICE"


/**
 * @brief 
 * 
 */
#define OFFER_SDP                           "OFFER_SDP"

/**
 * @brief 
 * 
 */
#define REQUEST_STREAM                      "REQUEST_STREAM"

/**
 * @brief 
 * 
 */
#define REQUEST_TYPE                        "RequestType"


/**
 * @brief 
 * 
 */
#define CONTENT                             "Content"




#endif