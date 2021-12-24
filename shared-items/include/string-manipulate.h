/**
 * @file string-manipulate.h
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __STRING_MANIPULATE_H__


char**          split                   (char *string, 
                                         char delimiter);

void            string_split_free       (char** base);

#define __STRING_MANIPULATE_H__
#endif