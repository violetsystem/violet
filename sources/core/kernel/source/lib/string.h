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

static inline int strncmp(const char *s1, const char *s2, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        if (s1[i] != s2[i]) {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
        if (s1[i] == '\0') {
            return 0;
        }
    }
    return 0;
}


#endif