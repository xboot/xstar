#ifndef __XSTAR_LIBX_DS_H__
#define __XSTAR_LIBX_DS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

struct ds_t {
	char * data;
	size_t length;
	size_t capacity;
};

static inline size_t ds_length(struct ds_t * ds)
{
	return ds ? ds->length : 0;
}

static inline size_t ds_capacity(struct ds_t * ds)
{
	return ds ? ds->capacity : 0;
}

static inline int ds_isempty(struct ds_t * ds)
{
	return (ds && (ds->length > 0)) ? 0 : 1;
}

static inline const char * ds_cstr(struct ds_t * ds)
{
	return ds ? ds->data : "";
}

struct ds_t * ds_alloc(void);
struct ds_t * ds_alloc_from_str(const char * str);
struct ds_t * ds_alloc_from_buf(const void * buf, int len);
void ds_free(struct ds_t * ds);

struct ds_t * ds_clone(struct ds_t * ds);
struct ds_t * ds_substr(struct ds_t * ds, size_t start, size_t len);
void ds_clear(struct ds_t * ds);
void ds_trim(struct ds_t * ds);

int ds_append(struct ds_t * ds, const char * str);
int ds_append_char(struct ds_t * ds, const char c);
int ds_prepend(struct ds_t * ds, const char * str);
int ds_insert(struct ds_t * ds, size_t pos, const char * str);
int ds_delete(struct ds_t * ds, size_t start, size_t len);

int ds_copy(struct ds_t * ds, const char * fmt, ...);
int ds_concat(struct ds_t * ds, const char * fmt, ...);

int ds_find(struct ds_t * ds, const char * sub);
int ds_rfind(struct ds_t * ds, const char * sub);
int ds_replace(struct ds_t * ds, const char * ostr, const char * nstr);
int ds_replace_first(struct ds_t * ds, const char * ostr, const char * nstr);
int ds_replace_last(struct ds_t * ds, const char * ostr, const char * nstr);

struct ds_iter_t {
	char * data;
	char * delim;
	char * pos;
};

struct ds_iter_t * ds_iter_alloc(struct ds_t * ds, const char * delim);
void ds_iter_free(struct ds_iter_t * iter);
char * ds_iter_next(struct ds_iter_t * iter);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_DS_H__ */
