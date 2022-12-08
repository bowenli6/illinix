#ifndef _STRING_H_
#define	_STRING_H_

#include <type.h>

size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strrev(const char *s);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strcat(char * __restrict dest, const char * __restrict src);
char *strncat(char * __restrict dest, const char * __restrict src, size_t n);
char *strcpy(char * __restrict dest, const char * __restrict src);
char *strncpy(char * __restrict dest, const char * __restrict src, size_t n);
char *stpcpy(char * __restrict dest, const char * __restrict src);
char *strdup(const char *s);


void *memset(void *s, int c, size_t n);
void *memcpy(void * __restrict dest, const void * __restrict src, size_t n);
void *memmove(void *dest, const void *src, size_t n);

char *itoa(unsigned int value, char *buf, int radix);
int atoi(const char *nptr);
#endif /* _STRING_H_ */
