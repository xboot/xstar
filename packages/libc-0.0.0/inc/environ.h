#ifndef __ENVIRON_H__
#define __ENVIRON_H__

#ifdef __cplusplus
extern "C" {
#endif

char * getenv(const char * name);
int putenv(const char * str);
int setenv(const char * name, const char * val, int overwrite);
int unsetenv(const char * name);
int clearenv(void);

#ifdef __cplusplus
}
#endif

#endif /* __ENVIRON_H__ */
