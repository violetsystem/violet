#ifndef _STRING_H
#define _STRING_H 1

#include <stddef.h>

static inline size_t strlen(const char *str) {
	register const char *s;
	for (s = str; *s; s++);
	return(s - str);
}

static inline int strcmp(const char *str1, const char *str2) {
    while (*str1 != '\0' && *str2 != '\0' && *str1 == *str2) {
        str1++;
        str2++;
    }
    
    if(*str1 == *str2) {
        return 0;
    }else if (*str1 < *str2) {
        return -1;
    }else {
        return 1;
    }
}

#endif