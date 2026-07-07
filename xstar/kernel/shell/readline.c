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
#include <kernel/command/command.h>
#include <kernel/shell/context.h>
#include <kernel/shell/readline.h>

enum esc_state_t {
	ESC_STATE_NORMAL,
	ESC_STATE_ESC,
	ESC_STATE_CSI,
};

struct rl_buf_t {
	char * prompt;
	uint32_t * buf;
	uint32_t * cut;
	int bsize;
	int pos;
	int len;

	enum esc_state_t state;
	int params[8];
	int num_params;
	char utf8[32];
	int usize;
};

static int ucs4_strlen(const uint32_t * s)
{
	const uint32_t * sc;

	if(!s)
		return 0;
	for(sc = s; *sc != '\0'; ++sc);
	return sc - s;
}

static void rl_space(struct rl_buf_t * rl, int len)
{
	for(int i = 0; i < len; i++)
		shell_printf(" ");
}

static void rl_print(struct rl_buf_t * rl, int from, int len)
{
	from = (from < 0) ? 0 : from;
	len = (len > rl->len) ? rl->len : len;

	if(len > 0)
	{
		char * utf8 = ucs4_to_utf8_alloc(rl->buf + from, len);
		if(utf8)
		{
			shell_printf("%s", utf8);
			xos_mem_free(utf8);
		}
	}
}

static void rl_cursor_save(struct rl_buf_t * rl)
{
	shell_printf("\033[s");
}

static void rl_cursor_restore(struct rl_buf_t * rl)
{
	shell_printf("\033[u");
}

static void rl_cursor_left(struct rl_buf_t * rl)
{
	if(rl->pos > 0)
	{
		rl->pos = rl->pos - 1;
		int w = ucs4_width(rl->buf[rl->pos]);
		if(w < 0)
			w = 1;
		for(int i = 0; i < w; i++)
			shell_printf("\033[D");
	}
}

static void rl_cursor_right(struct rl_buf_t * rl)
{
	if(rl->pos < rl->len)
	{
		int w = ucs4_width(rl->buf[rl->pos]);
		if(w < 0)
			w = 1;
		for(int i = 0; i < w; i++)
			shell_printf("\033[C");
		rl->pos = rl->pos + 1;
	}
}

static void rl_cursor_home(struct rl_buf_t * rl)
{
	while(rl->pos > 0)
	{
		rl_cursor_left(rl);
	}
}

static void rl_cursor_end(struct rl_buf_t * rl)
{
	while(rl->pos < rl->len)
	{
		rl_cursor_right(rl);
	}
}

static void rl_insert(struct rl_buf_t * rl, uint32_t * s)
{
	int len = ucs4_strlen(s);

	if(len <= 0)
		return;

	if(len + rl->len >= rl->bsize)
	{
		rl->bsize = rl->bsize * 2;
		uint32_t * p = xos_mem_realloc(rl->buf, sizeof(uint32_t) * rl->bsize);
		if(!p)
			return;
		rl->buf = p;
	}

	if(len + rl->len < rl->bsize)
	{
		xos_memmove(rl->buf + rl->pos + len, rl->buf + rl->pos, (rl->len - rl->pos + 1) * sizeof(uint32_t));
		xos_memmove(rl->buf + rl->pos, s, len * sizeof(uint32_t));

		rl->pos = rl->pos + len;
		rl->len = rl->len + len;
		rl_print(rl, rl->pos - len, len);
		rl_cursor_save(rl);
		rl_print(rl, rl->pos, rl->len - rl->pos);
		rl_cursor_restore(rl);
	}
}

static void rl_delete(struct rl_buf_t * rl, uint32_t len)
{
	if(len > rl->len - rl->pos)
		len = rl->len - rl->pos;

	if(rl->pos + len <= rl->len)
	{
		int n = 0;
		for(int i = 0; i < len; i++)
		{
			int w = ucs4_width(rl->buf[rl->pos + i]);
			if(w < 0)
				w = 0;
			n += w;
		}

		if(rl->cut)
			xos_mem_free(rl->cut);

		rl->cut = xos_mem_malloc((rl->len - rl->pos + 1) * sizeof(uint32_t));
		if(rl->cut)
		{
			xos_memcpy(rl->cut, rl->buf + rl->pos, (rl->len - rl->pos + 1) * sizeof(uint32_t));
			rl->cut[rl->len - rl->pos] = '\0';
		}

		xos_memmove(rl->buf + rl->pos, rl->buf + rl->pos + len, sizeof(uint32_t) * (rl->len - rl->pos + 1));
		rl->len = rl->len - len;
		rl_cursor_save(rl);
		rl_print(rl, rl->pos, rl->len - rl->pos);
		rl_space(rl, n);
		rl_cursor_restore(rl);
	}
}

static struct rl_buf_t * rl_alloc(const char * prompt)
{
	struct rl_buf_t * rl = xos_mem_malloc(sizeof(struct rl_buf_t));
	if(!rl)
		return NULL;

	rl->prompt = xos_strdup(prompt);
	rl->bsize = 128;
	rl->buf = xos_mem_malloc(sizeof(uint32_t) * rl->bsize);
	rl->cut = NULL;
	rl->pos = 0;
	rl->len = 0;

	rl->state = ESC_STATE_NORMAL;
	rl->num_params = 0;
	rl->usize = 0;

	if(!rl->buf)
	{
		if(rl->prompt)
			xos_mem_free(rl->prompt);
		xos_mem_free(rl);
		return NULL;
	}
	if(rl->prompt)
		shell_printf("%s", rl->prompt);
	return rl;
}

static void rl_free(struct rl_buf_t * rl)
{
	if(rl->prompt)
		xos_mem_free(rl->prompt);
	if(rl->cut)
		xos_mem_free(rl->cut);
	xos_mem_free(rl->buf);
	xos_mem_free(rl);
}

static int rl_getcode(struct rl_buf_t * rl, uint32_t * code)
{
	uint32_t cp;
	char * rest;
	int ch;

	if((ch = shell_getchar()) < 0)
		return FALSE;
	unsigned char c = ch;

	switch(rl->state)
	{
	case ESC_STATE_NORMAL:
		switch(c)
		{
		case 27:
			rl->state = ESC_STATE_ESC;
			break;

		case 127:				/* backspace */
			*code = 0x8;		/* ctrl-h */
			return TRUE;

		default:
			rl->utf8[rl->usize++] = c;
			if(utf8_to_ucs4(&cp, 1, (const char *)rl->utf8, rl->usize, (const char **)&rest) > 0)
			{
				rl->usize -= rest - rl->utf8;
				xos_memmove(rl->utf8, rest, rl->usize);

				*code = cp;
				return TRUE;
			}
			break;
		}
		break;

	case ESC_STATE_ESC:
		if(c == '[')
		{
			for(int i = 0; i < ARRAY_SIZE(rl->params); i++)
				rl->params[i] = 0;
			rl->num_params = 0;
			rl->state = ESC_STATE_CSI;
		}
		else
		{
			rl->state = ESC_STATE_NORMAL;
		}
		break;

	case ESC_STATE_CSI:
		if(c >= '0' && c <= '9')
		{
			if(rl->num_params < ARRAY_SIZE(rl->params))
			{
				rl->params[rl->num_params] = rl->params[rl->num_params] * 10 + c - '0';
			}
		}
		else
		{
			rl->num_params++;
			if(c == ';')
				break;

			rl->state = ESC_STATE_NORMAL;
			switch(c)
			{
			case 'A':				/* arrow up */
				*code = 0x10;		/* ctrl-p */
				return TRUE;

			case 'B':				/* arrow down */
				*code = 0xe			/* ctrl-n */;
				return TRUE;

			case 'C':				/* arrow right */
				*code = 0x6;		/* ctrl-f */
				return TRUE;

			case 'D':				/* arrow left */
				*code = 0x2;		/* ctrl-b */
				return TRUE;

			case '~':
				if(rl->num_params != 1)
					break;

				switch(rl->params[0])
				{
				case 1:				/* home */
					*code = 0x1;	/* ctrl-a */
					return TRUE;

				case 2:				/* insert */
					break;

				case 3:				/* delete */
					*code = 0x8;	/* ctrl-h */
					return TRUE;

				case 4:				/* end */
					*code = 0x5;	/* ctrl-e */
					return TRUE;

				case 5:				/* page up*/
					*code = 0x10;	/* ctrl-p */
					return TRUE;

				case 6:				/* page down*/
					*code = 0xe;	/* ctrl-n */
					return TRUE;

				default:
					break;
				}
				break;

			default:
				break;
			}
		}
		break;

	default:
		rl->state = ESC_STATE_NORMAL;
		break;
	}

	return FALSE;
}

struct rl_complete_file_data_t {
	char * bname;
	char * dname;
	struct slist_t * sl;
	int cnt;
};

static void rl_complete_file_cb(const char * dir, const char * name, void * data)
{
	struct rl_complete_file_data_t * rcfd = (struct rl_complete_file_data_t *)data;
	char path[CONFIG_XSTAR_MAX_PATH];

	if((xos_strcmp(name, ".") != 0) && (xos_strcmp(name, "..") != 0))
	{
		if(!xos_strncmp(rcfd->bname, name, xos_strlen(rcfd->bname)))
		{
			xos_snprintf(path, sizeof(path), "%s/%s", dir, name);
			if(xfs_isdir(shell_getxfs(), path))
			{
				slist_add(rcfd->sl, NULL, "%s/", name);
				rcfd->cnt++;
			}
			else if(xfs_isfile(shell_getxfs(), path))
			{
				slist_add(rcfd->sl, NULL, "%s", name);
				rcfd->cnt++;
			}
		}
	}
}

static void rl_complete_file(struct rl_buf_t * rl, char * utf8, char * p)
{
	struct rl_complete_file_data_t rcfd;
	struct slist_t * e;
	char fpath[CONFIG_XSTAR_MAX_PATH];
	int min = 0x7fffffff;
	int len = 0;

	if(shell_realpath(p, fpath) >= 0)
	{
		if((xos_strlen(p) > 0) && (p[xos_strlen(p) - 1] == '/'))
		{
			rcfd.bname = "";
			rcfd.dname = fpath;
		}
		else
		{
			rcfd.bname = path_basename(fpath);
			rcfd.dname = path_dirname(fpath);
		}
		int pl = xos_strlen(rcfd.bname);
		if(xfs_isdir(shell_getxfs(), rcfd.dname))
		{
			rcfd.sl = slist_alloc();
			rcfd.cnt = 0;
			xfs_walk(shell_getxfs(), rcfd.dname, rl_complete_file_cb, &rcfd);
			slist_natsort(rcfd.sl);
			if(rcfd.cnt > 0)
			{
				char * s = ((struct slist_t *)list_first_entry(&rcfd.sl->list, struct slist_t, list))->key;
				slist_for_each_entry(e, rcfd.sl)
				{
					int m = 0;
					while((s[m] == e->key[m]) && (s[m] != '\0'))
						m++;
					if(m < min)
						min = m;
				}
				if((rcfd.cnt == 1) || (min > pl))
				{
					if((e = (struct slist_t *)list_first_entry_or_null(&rcfd.sl->list, struct slist_t, list)))
					{
						uint32_t * ucs4;
						e->key[min] = 0;
						int l = utf8_to_ucs4_alloc(&e->key[pl], &ucs4, NULL);
						if(l > 0)
						{
							ucs4[l] = 0;
							rl_insert(rl, ucs4);
							xos_mem_free(ucs4);
						}
					}
				}
				else
				{
					slist_for_each_entry(e, rcfd.sl)
					{
						int l = xos_strlen(e->key) + 4;
						if(l > len)
							len = l;
					}
					int t = 80 / (len + 1);
					if(t == 0)
						t = 1;
					int i = 0;
					shell_printf("\r\n");
					slist_for_each_entry(e, rcfd.sl)
					{
						if(!(++i % t))
							shell_printf("%s\r\n", e->key);
						else
							shell_printf("%-*s", len, e->key);
					}
					if(i % t)
						shell_printf("\r\n");
					shell_printf("%s%s", rl->prompt ? rl->prompt : "", utf8);
				}
			}
			slist_free(rcfd.sl);
		}
	}
}

static void cmdcb(struct command_t * cmd, void * data)
{
	struct {
		struct slist_t * sl;
		char * p;
		int pl;
		int cnt;
	} * pdat = data;

	if(!xos_strncmp(pdat->p, cmd->name, pdat->pl))
	{
		slist_add(pdat->sl, cmd, "%s", cmd->name);
		pdat->cnt++;
	}
}

static void rl_complete_command(struct rl_buf_t * rl, char * utf8, char * p)
{
	struct slist_t * e;
	int min = INT_MAX;
	int len = 0;

	struct {
		struct slist_t * sl;
		char * p;
		int pl;
		int cnt;
	} pdat = { slist_alloc(), p, xos_strlen(p), 0 };
	command_foreach(cmdcb, &pdat);

	slist_sort(pdat.sl);
	if(pdat.cnt > 0)
	{
		char * s = ((struct slist_t *)list_first_entry(&pdat.sl->list, struct slist_t, list))->key;
		slist_for_each_entry(e, pdat.sl)
		{
			int m = 0;
			while((s[m] == e->key[m]) && (s[m] != '\0'))
				m++;
			if(m < min)
				min = m;
		}
		if((pdat.cnt == 1) || (min > pdat.pl))
		{
			if((e = (struct slist_t *)list_first_entry_or_null(&pdat.sl->list, struct slist_t, list)))
			{
				uint32_t * ucs4;
				e->key[min] = 0;
				int l = utf8_to_ucs4_alloc(&e->key[pdat.pl], &ucs4, NULL);
				if(l > 0)
				{
					ucs4[l] = 0;
					rl_insert(rl, ucs4);
					xos_mem_free(ucs4);
				}
			}
		}
		else
		{
			slist_for_each_entry(e, pdat.sl)
			{
				int l = xos_strlen(e->key) + 4;
				if(l > len)
					len = l;
			}
			int t = 80 / (len + 1);
			if(t == 0)
				t = 1;
			int i = 0;
			shell_printf("\r\n");
			slist_for_each_entry(e, pdat.sl)
			{
				if(!(++i % t))
					shell_printf("%s\r\n", e->key);
				else
					shell_printf("%-*s", len, e->key);
			}
			if(i % t)
				shell_printf("\r\n");
			shell_printf("%s%s", rl->prompt ? rl->prompt : "", utf8);
		}
	}
	slist_free(pdat.sl);
}

static void rl_complete(struct rl_buf_t * rl)
{
	if(rl->len > 0)
	{
		char * utf8 = ucs4_to_utf8_alloc(rl->buf, rl->len);
		char * q, * p = xos_strrchr(utf8, ';');
		if(!p)
			p = utf8;
		else
			p++;
		while(*p == ' ')
			p++;
		if(p && (xos_strchr(p, '.') || xos_strchr(p, '/')))
		{
			if((q = xos_strrchr(p, ' ')))
				p = ++q;
			rl_complete_file(rl, utf8, p);
		}
		else if((q = xos_strrchr(p, ' ')))
		{
			p = ++q;
			rl_complete_file(rl, utf8, p);
		}
		else
		{
			rl_complete_command(rl, utf8, p);
		}
		xos_mem_free(utf8);
	}
}

static int readline_handle(struct rl_buf_t * rl, uint32_t code)
{
	switch(code)
	{
	case 0x0:	/* null */
		break;

	case 0x1:	/* ctrl-a: to the start of the line */
		rl_cursor_home(rl);
		break;

	case 0x2:	/* ctrl-b: to the left */
		rl_cursor_left(rl);
		break;

	case 0x3:	/* ctrl-c: break the readline */
		rl_cursor_home(rl);
		rl_delete(rl, rl->len);
		return TRUE;

	case 0x4:	/* ctrl-d: delete the character underneath the cursor */
		if(rl->pos < rl->len)
			rl_delete(rl, 1);
		break;

	case 0x5:	/* ctrl-e: to the end of the line */
		rl_cursor_end(rl);
		break;

	case 0x6:	/* ctrl-f: to the right */
		rl_cursor_right(rl);
		break;

	case 0x7:	/* ctrl-g */
		break;

	case 0x8:	/* ctrl-h: backspace */
		if(rl->pos > 0)
		{
			rl_cursor_left(rl);
			rl_delete(rl, 1);
		}
		break;

	case 0x9: 	/* ctrl-i: tab */
		if(rl->len > 0)
			rl_complete(rl);
		break;

	case 0xb: 	/* ctrl-k: delete everything from the cursor to the end of the line */
		if(rl->pos < rl->len)
			rl_delete(rl, rl->len - rl->pos);
		break;

	case 0xc: 	/* ctrl-l */
		break;

	case 0xa:	/* ctrl-j: lf */
	case 0xd: 	/* ctrl-m: cr */
		if(rl->len > 0)
			shell_history_add(rl->buf, rl->len);
		return TRUE;

	case 0xe:	/* ctrl-n: the next history */
		rl_cursor_home(rl);
		rl_delete(rl, rl->len);
		rl_insert(rl, shell_history_next());
		break;

	case 0xf: 	/* ctrl-o */
		break;

	case 0x10:	/* ctrl-p: the previous history */
	{
		uint32_t * p = shell_history_prev();
		if(p)
		{
			rl_cursor_home(rl);
			rl_delete(rl, rl->len);
			rl_insert(rl, p);
		}
		break;
	}

	case 0x11: 	/* ctrl-q */
		break;

	case 0x12:	/* ctrl-r */
		break;

	case 0x13: 	/* ctrl-s */
		break;

	case 0x14: 	/* ctrl-t */
		break;

	case 0x15: 	/* ctrl-u: delete everything from the cursor back to the line start */
		if(rl->pos > 0)
		{
			uint32_t n = rl->pos;
			rl_cursor_home(rl);
			rl_delete(rl, n);
		}
		break;

	case 0x16:	/* ctrl-v */
		break;

	case 0x17: 	/* ctrl-w */
		break;

	case 0x18: 	/* ctrl-x */
		break;

	case 0x19:	/* ctrl-y: paste the killed text at the cursor position */
		if(rl->cut)
			rl_insert(rl, rl->cut);
		break;

	case 0x1a: 	/* ctrl-z */
		break;

	default:
	      rl_insert(rl, (uint32_t[]){ code, '\0' });
		break;
	}

	return FALSE;
}

char * shell_readline(const char * prompt)
{
	char * utf8 = NULL;

	struct rl_buf_t * rl = rl_alloc(prompt);
	if(!rl)
		return NULL;

	for(;;)
	{
		uint32_t code;
		if(rl_getcode(rl, &code))
		{
			if(readline_handle(rl, code))
			{
				shell_printf("\r\n");
				break;
			}
		}
		xos_thread_msleep(1);
	}

	if(rl->len > 0)
		utf8 = ucs4_to_utf8_alloc(rl->buf, rl->len);
	rl_free(rl);
	return utf8;
}

static void rl_insert_password(struct rl_buf_t * rl, uint32_t * s)
{
	int len = ucs4_strlen(s);

	if(len <= 0)
		return;

	if(len + rl->len >= rl->bsize)
	{
		rl->bsize = rl->bsize * 2;
		uint32_t * p = xos_mem_realloc(rl->buf, sizeof(uint32_t) * rl->bsize);
		if(!p)
			return;
		rl->buf = p;
	}

	if(len + rl->len < rl->bsize)
	{
		xos_memmove(rl->buf + rl->pos + len, rl->buf + rl->pos, (rl->len - rl->pos + 1) * sizeof(uint32_t));
		xos_memmove(rl->buf + rl->pos, s, len * sizeof(uint32_t));

		rl->pos = rl->pos + len;
		rl->len = rl->len + len;
		for(int i = 0; i < ((len > rl->len) ? rl->len : len); i++)
			shell_printf("*");
		rl_cursor_save(rl);
		for(int i = 0; i < ((rl->len - rl->pos > rl->len) ? rl->len : rl->len - rl->pos); i++)
			shell_printf("*");
		rl_cursor_restore(rl);
	}
}

static int password_handle(struct rl_buf_t * rl, uint32_t code)
{
	switch(code)
	{
	case 0x0:	/* null */
		break;

	case 0x1:	/* ctrl-a */
		break;

	case 0x2:	/* ctrl-b */
		break;

	case 0x3:	/* ctrl-c */
		break;

	case 0x4:	/* ctrl-d */
		break;

	case 0x5:	/* ctrl-e */
		break;

	case 0x6:	/* ctrl-f */
		break;

	case 0x7:	/* ctrl-g */
		break;

	case 0x8:	/* ctrl-h: backspace */
		if(rl->pos > 0)
		{
			rl_cursor_left(rl);
			rl_delete(rl, 1);
		}
		break;

	case 0x9: 	/* ctrl-i: tab */
		break;

	case 0xb: 	/* ctrl-k */
		break;

	case 0xc: 	/* ctrl-l */
		break;

	case 0xa:	/* ctrl-j: lf */
	case 0xd: 	/* ctrl-m: cr */
		return TRUE;

	case 0xe:	/* ctrl-n */
		break;

	case 0xf: 	/* ctrl-o */
		break;

	case 0x10:	/* ctrl-p */
		break;

	case 0x11: 	/* ctrl-q */
		break;

	case 0x12:	/* ctrl-r */
		break;

	case 0x13: 	/* ctrl-s */
		break;

	case 0x14: 	/* ctrl-t */
		break;

	case 0x15: 	/* ctrl-u */
		break;

	case 0x16:	/* ctrl-v */
		break;

	case 0x17: 	/* ctrl-w */
		break;

	case 0x18: 	/* ctrl-x */
		break;

	case 0x19:	/* ctrl-y */
		break;

	case 0x1a: 	/* ctrl-z */
		break;

	default:
		rl_insert_password(rl, (uint32_t[]){ code, '\0' });
		break;
	}

	return FALSE;
}

char * shell_password(const char * prompt)
{
	char * utf8 = NULL;

	struct rl_buf_t * rl = rl_alloc(prompt);
	if(!rl)
		return NULL;

	for(;;)
	{
		uint32_t code;
		if(rl_getcode(rl, &code))
		{
			if(password_handle(rl, code))
			{
				shell_printf("\r\n");
				break;
			}
		}
		xos_thread_msleep(1);
	}

	if(rl->len > 0)
		utf8 = ucs4_to_utf8_alloc(rl->buf, rl->len);
	rl_free(rl);
	return utf8;
}
