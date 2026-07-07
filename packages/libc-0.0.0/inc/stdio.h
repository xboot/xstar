#ifndef __STDIO_H__
#define __STDIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdarg.h>

enum {
	SEEK_SET		= 0,
	SEEK_CUR		= 1,
	SEEK_END		= 2,
};

typedef struct __libc_file_t {
	int fd;
} FILE;

#define stdin		&(FILE){0}
#define stdout		&(FILE){1}
#define stderr		&(FILE){2}

FILE * fopen(const char * path, const char * mode);
size_t fread(void * buf, size_t size, size_t count, FILE * f);
size_t fwrite(const void * buf, size_t size, size_t count, FILE * f);
int64_t fseek(FILE * f, int64_t off, int whence);
int64_t ftell(FILE * f);
int fflush(FILE * f);
int fclose(FILE * f);
int ferror(FILE * f);
int fprintf(FILE * f, const char * fmt, ...);
int fscanf(FILE * f, const char * fmt, ...);

int vsnprintf(char * buf, size_t n, const char * fmt, va_list ap);
int vasprintf(char ** s, const char * fmt, va_list ap);
int vprintf(const char * fmt, va_list ap);
int vsscanf(const char * buf, const char * fmt, va_list ap);
int snprintf(char * buf, size_t n, const char * fmt, ...);
int asprintf(char ** s, const char * fmt, ...);
int sprintf(char * buf, const char * fmt, ...);
int sscanf(const char * buf, const char * fmt, ...);
int printf(const char * fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* __STDIO_H__ */
