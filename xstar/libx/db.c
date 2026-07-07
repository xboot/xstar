/*
 * Copyright(c) Jianjun Jiang <8192542@qq.com>
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <xos/xos.h>
#include <libx/db.h>

static int db_grow(struct db_t * db, size_t needed)
{
	if(db->capacity < needed)
	{
		size_t ncap = (db->capacity << 1);
		while(ncap < needed)
			ncap <<= 1;
		unsigned char * ndata = xos_mem_realloc(db->data, ncap);
		if(!ndata)
			return 0;
		db->data = ndata;
		db->capacity = ncap;
	}
	return 1;
}

struct db_t * db_alloc(size_t size)
{
	struct db_t * db = xos_mem_malloc(sizeof(struct db_t));

	if(db)
	{
		if(size < 32)
			size = 32;
		if(size & (size - 1))
			size = roundup_pow_of_two(size);
		db->data = xos_mem_malloc(size);
		if(!db->data)
		{
			xos_mem_free(db);
			return NULL;
		}
		db->length = 0;
		db->capacity = size;
		return db;
	}
	return NULL;
}

void db_free(struct db_t * db)
{
	if(db)
	{
		xos_mem_free(db->data);
		xos_mem_free(db);
	}
}

void db_clear(struct db_t * db)
{
	if(db)
		db->length = 0;
}

int db_append(struct db_t * db, const void * buf, size_t len)
{
	if(db && buf && (len > 0))
	{
		if(!db_grow(db, db->length + len))
			return 0;
		xos_memcpy(db->data + db->length, buf, len);
		db->length += len;
		return 1;
	}
	return 0;
}

int db_append_byte(struct db_t * db, unsigned char byte)
{
	return db_append(db, &byte, 1);
}

int db_prepend(struct db_t * db, const void * buf, size_t len)
{
	if(db && buf && (len > 0))
	{
		if(!db_grow(db, db->length + len))
			return 0;
		xos_memmove(db->data + len, db->data, db->length);
		xos_memcpy(db->data, buf, len);
		db->length += len;
		return 1;
	}
	return 0;
}

int db_prepend_byte(struct db_t * db, unsigned char byte)
{
	return db_prepend(db, &byte, 1);
}

const char * db_safe_cstr(struct db_t * db)
{
	if(db && db_grow(db, db->length + 1))
	{
		db->data[db->length] = 0;
		return (const char *)db->data;
	}
	return "";
}
