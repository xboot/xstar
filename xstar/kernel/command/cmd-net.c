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

#include <driver/net/net.h>
#include <kernel/shell/context.h>
#include <kernel/shell/ctrlc.h>
#include <kernel/command/command.h>

struct srl_buf_t {
	char buf[1024];
	int len;
};

static int simple_readline(struct srl_buf_t * srl)
{
	int ch;

	if((ch = shell_getchar()) >= 0)
	{
		unsigned char c = ch;
		switch(c)
		{
		case 0x3:
			return -1;
		case 0xd:
			if(srl->len < sizeof(srl->buf) - 1)
			{
				srl->buf[srl->len++] = '\r';
				srl->buf[srl->len++] = '\n';
				shell_printf("\r\n");
			}
			return 1;
		default:
			if(srl->len < sizeof(srl->buf))
			{
				srl->buf[srl->len++] = c;
				shell_printf("%c", c);
			}
			break;
		}
	}
	return 0;
}

static void usage(void)
{
	struct device_t * pos, * n;

	shell_printf("usage:\r\n");
	shell_printf("    net <device> server <type> <port>        - Listen port for waiting connection\r\n");
	shell_printf("    net <device> client <type> <host> <port> - Connect to the remote server\r\n");

	shell_printf("supported device list:\r\n");
	list_for_each_entry_safe(pos, n, &__device_head[DEVICE_TYPE_NET], head)
	{
		shell_printf("    %s\r\n", pos->name);
	}
}

static int do_net(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, NULL, 3, 5))
	{
		usage();
		return -1;
	}

	const char * dev = sarg_at(&sarg, 0);
	const char * sub = sarg_at(&sarg, 1);
	struct net_t * net = search_net(dev);
	if(net)
	{
		if(!xos_strcmp(sub, "server"))
		{
			const char * type = sarg_at(&sarg, 2);
			const char * port = sarg_at(&sarg, 3);
			if(type && port && !sarg_at(&sarg, 4))
			{
				struct socket_listen_t * l = net_listen(net, type, xos_atoi(port));
				if(l)
				{
					while(1)
					{
						struct socket_connect_t * c = net_accept(l);
						if(c)
						{
							struct srl_buf_t srl;
							char buf[1024];
							int cknet = 1;
							if(xos_strstr(type, "udp"))
								cknet = 0;
							srl.len = 0;
							while(1)
							{
								if(cknet)
								{
									if(!net_status(c))
										break;
								}
								int r = simple_readline(&srl);
								if(r > 0)
								{
									net_write(c, srl.buf, srl.len);
									srl.len = 0;
								}
								else if(r < 0)
									break;
								int len = net_read(c, buf, sizeof(buf));
								if(len > 0)
								{
									for(int i = 0; i < len; i++)
									{
										if(xos_isprint(buf[i]) || (buf[i] == '\r') || (buf[i] == '\n') || (buf[i] == '\t') || (buf[i] == '\f'))
											shell_putchar(buf[i]);
										else
											shell_putchar('.');
									}
								}
							}
							net_close(c);
						}
						if(shell_ctrlc())
							break;
					}
					net_delete(l);
				}
				else
					shell_printf("Failed to listen '%s' with '%s' type\r\n", port, type);
			}
			else
				usage();
		}
		else if(!xos_strcmp(sub, "client"))
		{
			const char * type = sarg_at(&sarg, 2);
			const char * host = sarg_at(&sarg, 3);
			const char * port = sarg_at(&sarg, 4);
			if(type && host && port)
			{
				struct socket_connect_t * c = net_connect(net, type, host, xos_atoi(port));
				if(c)
				{
					struct srl_buf_t srl;
					char buf[1024];
					int cknet = 1;
					if(xos_strstr(type, "udp"))
						cknet = 0;
					srl.len = 0;
					while(1)
					{
						if(cknet)
						{
							if(!net_status(c))
								break;
						}
						int r = simple_readline(&srl);
						if(r > 0)
						{
							net_write(c, srl.buf, srl.len);
							srl.len = 0;
						}
						else if(r < 0)
							break;
						int len = net_read(c, buf, sizeof(buf));
						if(len > 0)
						{
							for(int i = 0; i < len; i++)
							{
								if(xos_isprint(buf[i]) || (buf[i] == '\r') || (buf[i] == '\n') || (buf[i] == '\t') || (buf[i] == '\f'))
									shell_putchar(buf[i]);
								else
									shell_putchar('.');
							}
						}
					}
					net_close(c);
				}
				else
					shell_printf("Failed to connect server '%s:%s' with '%s' type\r\n", host, port, type);
			}
			else
				usage();
		}
		else
			usage();
	}
	else
		usage();
	return 0;
}

static struct command_t cmd_net = {
	.name	= "net",
	.desc	= "network device transport tool",
	.usage	= usage,
	.exec	= do_net,
};

static void net_cmd_init(void)
{
	register_command(&cmd_net);
}

static void net_cmd_exit(void)
{
	unregister_command(&cmd_net);
}

command_initcall(net_cmd_init);
command_exitcall(net_cmd_exit);
