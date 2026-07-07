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

#include <xstar.h>
#include <xos/xos.h>
#include <kernel/command/command.h>
#include <kernel/shell/context.h>
#include <kernel/shell/readline.h>
#include <kernel/shell/shell.h>

static char ** shell_tokenize(char * cmd)
{
	char * p = cmd;
	int pos = 0;
	int bufsz = 16;
	char ** tokens = xos_mem_malloc(bufsz * sizeof(char *));

	if(tokens)
	{
		while(*p)
		{
			while(xos_isblank(*p))
				p++;
			if(!*p)
				break;
			char * tok;
			if(*p == '"')
			{
				tok = ++p;
				while(*p && (*p != '"'))
					p++;
				if(*p != '"')
				{
					xos_mem_free(tokens);
					return NULL;
				}
				*p++ = '\0';
				if(*p && !xos_isblank(*p))
				{
					xos_mem_free(tokens);
					return NULL;
				}
			}
			else if(*p == '\'')
			{
				tok = ++p;
				while(*p && (*p != '\''))
					p++;
				if(*p != '\'')
				{
					xos_mem_free(tokens);
					return NULL;
				}
				*p++ = '\0';
				if(*p && !xos_isblank(*p))
				{
					xos_mem_free(tokens);
					return NULL;
				}
			}
			else
			{
				tok = p;
				while(*p && !xos_isblank(*p))
				{
					if((*p == '"') || (*p == '\''))
					{
						xos_mem_free(tokens);
						return NULL;
					}
					p++;
				}
				if(*p)
					*p++ = '\0';
			}
			tokens[pos++] = tok;
			if(pos >= bufsz)
			{
				bufsz <<= 1;
				char ** tmp = xos_mem_realloc(tokens, bufsz * sizeof(char *));
				if(!tmp)
				{
					xos_mem_free(tokens);
					return NULL;
				}
				tokens = tmp;
			}
		}
		tokens[pos] = NULL;
		return tokens;
	}
	return NULL;
}

static char * shell_splitcmd(char ** s)
{
	char * begin = *s;
	char * p = begin;
	int quote = 0;
	int squote = 0;

	if(!begin)
		return NULL;
	while(*p)
	{
		if(squote)
		{
			if(*p == '\'')
				squote = 0;
		}
		else if(quote)
		{
			if(*p == '"')
				quote = 0;
		}
		else if(*p == '"')
			quote = 1;
		else if(*p == '\'')
			squote = 1;
		else if(*p == ';')
		{
			*p++ = '\0';
			*s = p;
			return begin;
		}
		p++;
	}
	*s = NULL;
	return begin;
}

int shell_system(const char * cmdline)
{
	char * str = xos_strdup(cmdline);
	int ret = 0;

	if(str)
	{
		char * p = str, * r, * cmd;
		while((r = shell_splitcmd(&p)) != NULL)
		{
			cmd = xos_strim(r);
			if(cmd && (*cmd != '\0'))
			{
				char ** argv = shell_tokenize(cmd);
				if(argv)
				{
					int argc = 0;
					while(argv[argc] != NULL)
						argc++;
					if(argc > 0)
					{
						struct command_t * c = search_command(argv[0]);
						if(c)
						{
							ret = c->exec(argc, argv);
						}
						else
						{
							ret = -1;
							shell_printf(" could not found '%s' command\r\n", argv[0]);
						}
					}
					xos_mem_free(argv);
				}
				else
				{
					ret = -1;
					shell_printf(" syntax error\r\n");
				}
			}
		}
		xos_mem_free(str);
	}
	return ret;
}

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    shell\r\n");
}

static int do_shell(int argc, char ** argv)
{
	const char * password = CONFIG_XSTAR_SHELL_PASSWORD;
	if(password && (xos_strlen(password) == 64))
	{
		while(1)
		{
			char * p = shell_password("Password: ");
			if(p)
			{
				uint8_t digest[SHA256_DIGEST_SIZE];
				char buf[SHA256_DIGEST_SIZE * 2];
				sha256_hash(p, xos_strlen(p), digest);
				for(int i = 0; i < sizeof(buf); i++)
				{
					char v = (digest[i >> 1] >> ((i & 0x1) ? 0 : 4)) & 0xf;
					buf[i] = ((v < 10) ? (v + '0') : (v - 10 + 'a'));
				}
				xos_mem_free(p);
				if(xos_memcmp(buf, password, sizeof(buf)) == 0)
					break;
			}
			shell_printf("Sorry, please try again.\r\n");
		}
	}

	shell_printf("%s\r\n", xstar_clogo());
	shell_printf("%s\r\n", xstar_banner());
	while(1)
	{
		char prompt[256];
		xos_snprintf(prompt, sizeof(prompt), "xstar: %s# ", shell_getcwd());
		char * p = shell_readline(prompt);
		if(p)
		{
			shell_system(p);
			xos_mem_free(p);
		}
	}
	return 0;
}

static struct command_t cmd_shell = {
	.name	= "shell",
	.desc	= "shell command interpreter",
	.usage	= usage,
	.exec	= do_shell,
};

static void shell_cmd_init(void)
{
	register_command(&cmd_shell);
}

static void shell_cmd_exit(void)
{
	unregister_command(&cmd_shell);
}

command_initcall(shell_cmd_init);
command_exitcall(shell_cmd_exit);
