/**
 * @file message-form.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __MESSAGE_FORM_H__
#define __MESSAGE_FORM_H__

#include <glib.h>
#include <json-glib/json-glib.h>

#include <opcode.h>
#include <module-code.h>



/**
 * @brief Get the string from json object object
 * @param object 
 * @return gchar* 
 */
gchar*              get_string_from_json_object     (JsonObject* object);

/**
 * @brief Get the json object from string object
 * create json object from string
 * @param string string to parse
 * @param error error pointer to pointer, must be set to null before set
 * @param parser json parser, remember to init and free before and after use
 * @return JsonObject* json object
 */
JsonObject*            get_json_object_from_string  (gchar* string,
                                                    GError** error,
                                                    JsonParser* parser);



/**
 * @brief Get the json object from file object
 * handy function for get json object from a file 
 * @param file_name 
 * @param error 
 * @return JsonObject* result 
 */
JsonObject*            get_json_object_from_file   (gchar* file_name,
                                                    GError** error);

#endif