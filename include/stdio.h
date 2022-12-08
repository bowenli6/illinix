#ifndef _STDIO_H_
#define	_STDIO_H_

#define stdin 0
#define stdout 1
#define stderr 2

int printf(const char *format, ...);
int puts(char *s);
void putc(unsigned char c);
int sprintf(char *str, const char *format, ...);
int fprintf(int fd, const char *format, ...);
int scanf(const char *format, ...);
int fputs(int fd, const char *s);
char *fgets(char *s, int size, int stream);

#endif /* _STDIO_H_ */
