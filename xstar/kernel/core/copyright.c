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

#include <kernel/core/copyright.h>

static const char * __copyright_dummy_uniqueid(void)
{
	return "0123456789";
}

static int __copyright_dummy_keygen(const char * msg, void * key, int maxlen)
{
	if(key && (maxlen > 0))
	{
		uint8_t digest[SHA256_DIGEST_SIZE];
		int len = XMIN((int)sizeof(digest), maxlen);
		sha256_hash(msg, xos_strlen(msg), digest);
		xos_memcpy(key, digest, len);
		return len;
	}
	return 0;
}

static int __copyright_dummy_verify(void)
{
	return 1;
}

static struct copyright_t __copyright_dummy = {
	.uniqueid	= __copyright_dummy_uniqueid,
	.keygen		= __copyright_dummy_keygen,
	.verify		= __copyright_dummy_verify,
};
static struct copyright_t * __copyright = &__copyright_dummy;

const char * copyright_uniqueid(void)
{
	const char * id = __copyright->uniqueid();
	if(id)
		return id;
	return __copyright_dummy_uniqueid();
}

int copyright_keygen(const char * msg, void * key, int maxlen)
{
	int len = __copyright->keygen(msg, key, maxlen);
	if(len > 0)
		return len;
	return __copyright_dummy_keygen(msg, key, maxlen);
}

int copyright_verify(void)
{
	return __copyright->verify();
}

void register_copyright(struct copyright_t * c)
{
	if(c && c->uniqueid && c->keygen && c->verify)
	{
		if(__copyright == &__copyright_dummy)
			__copyright = c;
	}
}
