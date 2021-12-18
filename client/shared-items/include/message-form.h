#ifndef __MESSAGE_FORM_H__
#define __MESSAGE_FORM_H__

#include <glib.h>
#include <json-glib/json-glib.h>

#include <module-code.h>

JsonObject*            message_init              (Module from,
                                                 Module to,
                                                 gint opcode,
                                                 JsonObject* data);

/**
 * @brief Get the string from json object object
 * 
 * @param object 
 * @return gchar* 
 */
gchar*              get_string_from_json_object (JsonObject* object);

/**
 * @brief Get the json object from string object
 * 
 * @param string 
 * @param error 
 * @param parser 
 * @return Message* 
 */
JsonObject*            get_json_object_from_string(gchar* string,
                                                GError** error,
                                                JsonParser* parser);

/**
 * @brief 
 * 
 * @param from 
 * @param to 
 * @param opcode 
 * @return JsonObject* 
 */
JsonObject*            empty_message_init          (Module from,
			                                    Module to,
			                                    gint opcode);


JsonObject*            get_json_object_from_file   (gchar* file_name,
                                                 GError** error);

#endif