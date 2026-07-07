#ifndef __XSTAR_LIBX_PATH_H__
#define __XSTAR_LIBX_PATH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

static inline int is_absolute_path(const char * path)
{
	if(path)
	{
		if(*path == '/')
			return 1;
		else if(((*path) >= 'A' && (*path) <= 'Z') || ((*path) >= 'a' && (*path) <= 'z'))
		{
			if((path[1] == ':') && (path[2] == '/' || path[2] == '\\'))
				return 1;
		}
	}
	return 0;
}

static inline int is_relative_path(const char * path)
{
	if(!path || !*path)
		return 0;
	return !is_absolute_path(path);
}

char * path_basename(char * path);
char * path_dirname(char * path);
const char * path_fileext(const char * filename);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_PATH_H__ */
