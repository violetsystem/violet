#ifndef _STRING_H
#define _STRING_H 1

#include <stddef.h>

static inline size_t strlen(const char *str){
	register const char *s;
	for (s = str; *s; s++);
	return(s - str);
}

#endif