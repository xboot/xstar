#ifndef __XSTAR_LIBX_DB_H__
#define __XSTAR_LIBX_DB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

struct db_t {
	unsigned char * data;
	size_t length;
	size_t capacity;
};

static inline size_t db_length(struct db_t * db)
{
	return db ? db->length : 0;
}

static inline size_t db_capacity(struct db_t * db)
{
	return db ? db->capacity : 0;
}

static inline unsigned char * db_data(struct db_t * db)
{
	return db ? db->data : NULL;
}

struct db_t * db_alloc(size_t size);
void db_free(struct db_t * db);
void db_clear(struct db_t * db);
int db_append(struct db_t * db, const void * buf, size_t len);
int db_append_byte(struct db_t * db, unsigned char byte);
int db_prepend(struct db_t * db, const void * buf, size_t len);
int db_prepend_byte(struct db_t * db, unsigned char byte);
const char * db_safe_cstr(struct db_t * db);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_DB_H__ */
