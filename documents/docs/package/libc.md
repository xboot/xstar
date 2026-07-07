# libc (baremetal)

Baremetal C 标准库实现，提供标准 C 语言基础运行时支持。适用于无操作系统的裸机环境，替代宿主系统的 libc。

包含头文件：`assert.h`、`ctype.h`、environ.h、`errno.h`、`exit.h`、`locale.h`、`malloc.h`、`stdarg.h`、`stddef.h`、`stdio.h`、`stdlib.h`、`string.h`、`time.h`。

## assert.h

断言宏，定义 `NDEBUG` 时禁用。

```c
void __assert_fail(const char * expr, const char * file, int line, const char * func);
```

## ctype.h

字符分类与转换。

```c
int isalnum(int c);
int isalpha(int c);
int isascii(int c);
int isblank(int c);
int iscntrl(int c);
int isdigit(int c);
int isgraph(int c);
int islower(int c);
int isprint(int c);
int ispunct(int c);
int isspace(int c);
int isupper(int c);
int isxdigit(int c);
int toascii(int c);
int tolower(int c);
int toupper(int c);
```

## environ.h

环境变量操作。

```c
char * getenv(const char * name);
int putenv(const char * str);
int setenv(const char * name, const char * val, int overwrite);
int unsetenv(const char * name);
int clearenv(void);
```

## errno.h

错误码定义（`ENOERR` ~ `ENODEV` 共 34 个）及线程安全的 `errno` 宏。

```c
volatile int * __errno_location(void);
char * strerror(int e);
```

## exit.h

进程退出。

```c
void abort(void);
void exit(int code);
```

常量：`EXIT_SUCCESS` (0)、`EXIT_FAILURE` (1)。

## locale.h

区域设置。

```c
char * setlocale(int category, const char * locale);
struct lconv * localeconv(void);
```

区域类别：`LC_CTYPE`、`LC_NUMERIC`、`LC_TIME`、`LC_COLLATE`、`LC_MONETARY`、`LC_MESSAGES`、`LC_ALL`。

## malloc.h

动态内存分配。

```c
void * malloc(size_t size);
void * memalign(size_t align, size_t size);
void * realloc(void * ptr, size_t size);
void * calloc(size_t nmemb, size_t size);
void free(void * ptr);
```

## stdarg.h

可变参数处理（基于编译器内建）。

```c
typedef __builtin_va_list va_list;
va_start(v, l)
va_arg(v, l)
va_end(v)
va_copy(d, s)
```

## stddef.h

基础定义。

```c
#define NULL ((void *)0)
```

## stdio.h

标准 I/O。

```c
typedef struct __libc_file_t { int fd; } FILE;
#define stdin  (&(FILE){0})
#define stdout (&(FILE){1})
#define stderr (&(FILE){2})

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
```

`whence` 取值：`SEEK_SET` (0)、`SEEK_CUR` (1)、`SEEK_END` (2)。

## stdlib.h

通用工具函数。

```c
int abs(int n);
long labs(long n);
long long llabs(long long n);
int atoi(const char * nptr);
long atol(const char * nptr);
long long atoll(const char * nptr);
double atof(const char * nptr);
long strtol(const char * nptr, char ** endptr, int base);
long long strtoll(const char * nptr, char ** endptr, int base);
unsigned long strtoul(const char * nptr, char ** endptr, int base);
unsigned long long strtoull(const char * nptr, char ** endptr, int base);
double strtod(const char * nptr, char ** endptr);
intmax_t strtoimax(const char * nptr, char ** endptr, int base);
uintmax_t strtoumax(const char * nptr, char ** endptr, int base);
intmax_t strntoimax(const char * nptr, char ** endptr, int base, size_t n);
uintmax_t strntoumax(const char * nptr, char ** endptr, int base, size_t n);
void * bsearch(const void * key, const void * base, size_t nmemb, size_t size, int (*cmp)(const void *, const void *));
void qsort(void * aa, size_t n, size_t es, int (*cmp)(const void *, const void *));
int rand(void);
void srand(unsigned int seed);
```

常量：`RAND_MAX` (0x7fffffff)。

## string.h

字符串与内存操作。

```c
char * strcpy(char * dest, const char * src);
char * strncpy(char * dest, const char * src, size_t n);
size_t strlcpy(char * dest, const char * src, size_t n);
char * strcat(char * dest, const char * src);
char * strncat(char * dest, const char * src, size_t n);
size_t strlcat(char * dest, const char * src, size_t n);
size_t strlen(const char * s);
size_t strnlen(const char * s, size_t n);
int strcmp(const char * s1, const char * s2);
int strncmp(const char * s1, const char * s2, size_t n);
int strnicmp(const char * s1, const char * s2, size_t n);
int strcasecmp(const char * s1, const char * s2);
int strncasecmp(const char * s1, const char * s2, size_t n);
int strcoll(const char * s1, const char * s2);
char * strchr(const char * s, int c);
char * strrchr(const char * s, int c);
char * strnchr(const char * s, size_t n, int c);
size_t strspn(const char * s, const char * accept);
size_t strcspn(const char * s, const char * reject);
char * strpbrk(const char * s1, const char * s2);
char * strstr(const char * s1, const char * s2);
char * strnstr(const char * s1, const char * s2, size_t n);
char * strdup(const char * s);
char * strndup(const char * s, size_t n);
char * strsep(char ** s, const char * ct);
void * memset(void * s, int c, size_t n);
void * memcpy(void * dest, const void * src, size_t len);
void * memmove(void * dest, const void * src, size_t n);
void * memchr(const void * s, int c, size_t n);
int memcmp(const void * s1, const void * s2, size_t n);
```

## time.h

时间与日期。

```c
typedef uint64_t clock_t;
typedef int64_t time_t;
#define CLOCKS_PER_SEC (1000000000ULL)

struct tm {
    int tm_sec, tm_min, tm_hour;
    int tm_mday, tm_mon, tm_year;
    int tm_wday, tm_yday, tm_isdst;
};

struct timeval {
    time_t tv_sec;
    time_t tv_usec;
};

clock_t clock(void);
time_t time(time_t * t);
time_t timegm(struct tm * tm);
time_t mktime(struct tm * tm);
double difftime(time_t, time_t);
struct tm * gmtime(const time_t * t);
struct tm * gmtime_r(const time_t * t, struct tm * tm);
struct tm * localtime(const time_t * t);
struct tm * localtime_r(const time_t * t, struct tm * tm);
char * asctime(const struct tm * tm);
char * asctime_r(const struct tm * tm, char * buf);
char * ctime(const time_t * t);
char * ctime_r(const time_t * t, char * buf);
size_t strftime(char * s, size_t max, const char * fmt, const struct tm * t);
int gettimeofday(struct timeval * tv, void * tz);
int settimeofday(struct timeval * tv, void * tz);
int timezone(const char * tz);
```
