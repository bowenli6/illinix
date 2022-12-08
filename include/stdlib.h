#ifndef _STDLIB_H_
#define	_STDLIB_H_

#include <type.h>

void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

void exit(int status);
pid_t Fork(void);
void Execv(const char *pathname, char *const argv[]);
void Waitpid(pid_t pid, int *wstatus);

#endif /* _STDLIB_H_ */
