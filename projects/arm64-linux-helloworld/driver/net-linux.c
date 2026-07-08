#include <linux/linux.h>

struct net_linux_pdata_t {
	char * iface;
};

static struct socket_listen_t * net_linux_listen(struct net_t * net, const char * type, int port)
{
	void * lctx;

	lctx = linux_socket_listen(type, port);
	if(lctx)
		return socket_listen_alloc(net, lctx);
	return NULL;
}

static struct socket_connect_t * net_linux_accept(struct socket_listen_t * l)
{
	void * cctx;

	cctx = linux_socket_accept(l->priv);
	if(cctx)
		return socket_connect_alloc(l->net, cctx);
	return NULL;
}

static struct socket_connect_t * net_linux_connect(struct net_t * net, const char * type, const char * host, int port)
{
	void * cctx;

	cctx = linux_socket_connect(type, host, port);
	if(cctx)
		return socket_connect_alloc(net, cctx);
	return NULL;
}

static int net_linux_read(struct socket_connect_t * c, void * buf, int count)
{
	return linux_socket_read(c->priv, buf, count);
}

static int net_linux_write(struct socket_connect_t * c, void * buf, int count)
{
	return linux_socket_write(c->priv, buf, count);
}

static int net_linux_status(struct socket_connect_t * c)
{
	return linux_socket_status(c->priv);
}

static void net_linux_close(struct socket_connect_t * c)
{
	linux_socket_close(c->priv);
	socket_connect_free(c);
}

static void net_linux_delete(struct socket_listen_t * l)
{
	linux_socket_delete(l->priv);
	socket_listen_free(l);
}

static int net_linux_ioctl(struct net_t * net, const char * cmd, void * arg)
{
	struct net_linux_pdata_t * pdat = (struct net_linux_pdata_t *)net->priv;

	switch(shash(cmd))
	{
	case 0xccaf0ac8: /* "net-get-type" */
		if(arg)
		{
			strcpy((char *)arg, "sandbox");
			return 0;
		}
		break;

	case 0x74c961bf: /* "net-get-ip" */
		if(arg)
		{
			if(linux_socket_get_ip(pdat->iface, arg))
				return 0;
		}
		break;

	case 0x0df5a917: /* "net-get-mac" */
		if(arg)
		{
			if(linux_socket_get_mac(pdat->iface, arg))
				return 0;
		}
		break;

	default:
		break;
	}
	return -1;
}

static struct device_t * net_linux_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct net_linux_pdata_t * pdat;
	struct net_t * net;
	struct device_t * dev;

	pdat = xos_mem_malloc(sizeof(struct net_linux_pdata_t));
	if(!pdat)
		return NULL;

	net = xos_mem_malloc(sizeof(struct net_t));
	if(!net)
		return NULL;

	pdat->iface = xos_strdup(dt_read_string(n, "interface", "eth0"));

	net->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	net->listen = net_linux_listen;
	net->accept = net_linux_accept;
	net->connect = net_linux_connect;
	net->read = net_linux_read;
	net->write = net_linux_write;
	net->status = net_linux_status;
	net->close = net_linux_close;
	net->delete = net_linux_delete;
	net->ioctl = net_linux_ioctl;
	net->priv = pdat;

	if(!(dev = register_net(net, drv)))
	{
		free_device_name(net->name);
		xos_mem_free(net->priv);
		xos_mem_free(net);
		return NULL;
	}
	return dev;
}

static void net_linux_remove(struct device_t * dev)
{
	struct net_t * net = (struct net_t *)dev->priv;

	if(net)
	{
		unregister_net(net);
		free_device_name(net->name);
		xos_mem_free(net->priv);
		xos_mem_free(net);
	}
}

static void net_linux_suspend(struct device_t * dev)
{
}

static void net_linux_resume(struct device_t * dev)
{
}

static struct driver_t net_linux = {
	.name		= "net-linux",
	.probe		= net_linux_probe,
	.remove		= net_linux_remove,
	.suspend	= net_linux_suspend,
	.resume		= net_linux_resume,
};

static void net_linux_driver_init(void)
{
	register_driver(&net_linux);
}

static void net_linux_driver_exit(void)
{
	unregister_driver(&net_linux);
}

driver_initcall(net_linux_driver_init);
driver_exitcall(net_linux_driver_exit);
