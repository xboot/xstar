#include <xos/xos.h>
#include <environ.h>

struct environ_t {
	char * content;
	struct environ_t * prev;
	struct environ_t * next;
};

static struct environ_t __xenviron = {
	.content = NULL,
	.prev = &__xenviron,
	.next = &__xenviron,
};

static int __put_env(char * str, size_t len, int overwrite)
{
	struct environ_t * xenv = &__xenviron;
	struct environ_t * env;
	struct environ_t * p;

	if(!xenv)
		return -1;

	for(p = xenv->next; p != xenv; p = p->next)
	{
		if(p->content && !xos_strncmp(p->content, str, len))
		{
			if(!overwrite)
			{
				xos_mem_free(str);
			}
			else
			{
				xos_mem_free(p->content);
				p->content = str;
			}
			return 0;
		}
	}

	env = xos_mem_malloc(sizeof(struct environ_t));
	if(!env)
		return -1;

	env->content = str;
	env->prev = xenv->prev;
	env->next = xenv;
	xenv->prev->next = env;
	xenv->prev = env;

	return 0;
}

char * getenv(const char * name)
{
	struct environ_t * xenv = &__xenviron;
	struct environ_t * p;
	int len;

	if(!xenv || !xenv->content)
		return NULL;

	len = xos_strlen(name);
	for(p = xenv->next; p != xenv; p = p->next)
	{
		if(!xos_strncmp(name, p->content, len) && (p->content[len] == '='))
			return p->content + (len + 1);
	}
	return NULL;
}

int putenv(const char * str)
{
	char * s;
	const char * e, * z;

	if(!str)
		return -1;

	e = NULL;
	for(z = str; *z; z++)
	{
		if(*z == '=')
			e = z;
	}
	if(!e)
		return -1;

	s = xos_strdup(str);
	if(!s)
		return -1;

	return __put_env(s, e - str, 1);
}

int setenv(const char * name, const char * val, int overwrite)
{
	const char *z;
	char *s;
	size_t l1, l2;

	if(!name || !name[0])
		return -1;

	l1 = 0;
	for(z = name; *z; z++)
	{
		l1++;
		if(*z == '=')
			return -1;
	}

	l2 = xos_strlen(val);

	s = xos_mem_malloc(l1 + l2 + 2);
	if(!s)
		return -1;

	xos_memcpy(s, name, l1);
	s[l1] = '=';
	xos_memcpy(s + l1 + 1, val, l2 + 1);

	return __put_env(s, l1 + 1, overwrite);
}

int unsetenv(const char * name)
{
	struct environ_t * xenv = &__xenviron;
	struct environ_t * p;
	size_t len;
	const char * z;

	if(!name || !name[0])
		return -1;

	len = 0;
	for(z = name; *z; z++)
	{
		len++;
		if(*z == '=')
			return -1;
	}

	if(!xenv || !xenv->content)
		return 0;

	for(p = xenv->next; p != xenv; p = p->next)
	{
		if(!xos_strncmp(name, p->content, len) && (p->content[len] == '='))
		{
			p->next->prev = p->prev;
			p->prev->next = p->next;

			xos_mem_free(p->content);
			xos_mem_free(p);
			break;
		}
	}
	return 0;
}

int clearenv(void)
{
	struct environ_t * xenv = &__xenviron;
	struct environ_t * p, * q;

	if (!xenv || !xenv->content)
		return -1;

	for(p = xenv->next; p != xenv;)
	{
		q = p;
		p = p->next;

		q->next->prev = q->prev;
		q->prev->next = q->next;
		xos_mem_free(q->content);
		xos_mem_free(q);
	}
	return 0;
}
