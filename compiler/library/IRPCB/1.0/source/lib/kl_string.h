/*
 * kl_string.h
 *
 *  Created on: 5 дек. 2020 г.
 *      Author: Kreyl
 */

#ifndef KL_STRING_H_
#define KL_STRING_H_

int kl_strcasecmp(const char *s1, const char *s2);
char* kl_strtok(char* s, const char* delim, char**PLast);
int kl_sscanf(const char* s, const char* format, ...);
int kl_strlen(const char* s);
long kl_strtol(const char* nptr, char** endptr, int base);


#endif // KL_STRING_H_
