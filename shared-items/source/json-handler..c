/**
 * @file json-handler.hc
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-09
 * 
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <json-handler.h>







gchar*
get_string_from_json_object(JsonObject* object)
{
    JsonNode* root;
    JsonGenerator* generator;
    gchar* text;

    /* Make it the root node */
    root =      json_node_init_object(json_node_alloc(), object);
    generator = json_generator_new();
    json_generator_set_root(generator, root);
    text =      json_generator_to_data(generator, NULL);

    /* Release everything */
    g_object_unref(generator);
    json_node_free(root);
    return text;
}



JsonObject*
get_json_object_from_string(gchar* string, 
                            GError** error,
                            JsonParser* parser)
{
    JsonNode* root;
	json_parser_load_from_data(parser, string, -1, error);
	root = json_parser_get_root(parser);
    if(!root)
        return NULL;
    return json_node_get_object(root);
}


JsonObject*
get_json_object_from_file(gchar* file_name, 
                          GError** error)
{
    JsonNode* root;
    JsonParser* parser = json_parser_new();
	json_parser_load_from_file(parser, file_name, error);
	root = json_parser_get_root(parser);
    if(!root)
        return NULL;
    return json_node_get_object(root);
}