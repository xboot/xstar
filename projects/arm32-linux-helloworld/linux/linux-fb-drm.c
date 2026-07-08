#include <linux/linux.h>

typedef struct _drmModeRes {
	int count_fbs;
	uint32_t * fbs;
	int count_crtcs;
	uint32_t * crtcs;
	int count_connectors;
	uint32_t * connectors;
	int count_encoders;
	uint32_t * encoders;
	uint32_t min_width, max_width;
	uint32_t min_height, max_height;
} drmModeRes, *drmModeResPtr;

typedef enum {
	DRM_MODE_CONNECTED					= 1,
	DRM_MODE_DISCONNECTED				= 2,
	DRM_MODE_UNKNOWNCONNECTION			= 3,
} drmModeConnection;

typedef enum {
	DRM_MODE_SUBPIXEL_UNKNOWN			= 1,
	DRM_MODE_SUBPIXEL_HORIZONTAL_RGB	= 2,
	DRM_MODE_SUBPIXEL_HORIZONTAL_BGR	= 3,
	DRM_MODE_SUBPIXEL_VERTICAL_RGB		= 4,
	DRM_MODE_SUBPIXEL_VERTICAL_BGR		= 5,
	DRM_MODE_SUBPIXEL_NONE				= 6,
} drmModeSubPixel;

typedef struct _drmModeModeInfo {
	uint32_t clock;
	uint16_t hdisplay, hsync_start, hsync_end, htotal, hskew;
	uint16_t vdisplay, vsync_start, vsync_end, vtotal, vscan;
	uint32_t vrefresh;
	uint32_t flags;
	uint32_t type;
	char name[32];
} drmModeModeInfo, *drmModeModeInfoPtr;

typedef struct _drmModeConnector {
	uint32_t connector_id;
	uint32_t encoder_id;
	uint32_t connector_type;
	uint32_t connector_type_id;
	drmModeConnection connection;
	uint32_t mmWidth, mmHeight;
	drmModeSubPixel subpixel;
	int count_modes;
	drmModeModeInfoPtr modes;
	int count_props;
	uint32_t * props;
	uint64_t * prop_values;
	int count_encoders;
	uint32_t * encoders;
} drmModeConnector, *drmModeConnectorPtr;

typedef struct _drmModeCrtc {
	uint32_t crtc_id;
	uint32_t buffer_id;
	uint32_t x, y;
	uint32_t width, height;
	int mode_valid;
	drmModeModeInfo mode;
	int gamma_size;
} drmModeCrtc, *drmModeCrtcPtr;

typedef struct _drmModeEncoder {
	uint32_t encoder_id;
	uint32_t encoder_type;
	uint32_t crtc_id;
	uint32_t possible_crtcs;
	uint32_t possible_clones;
} drmModeEncoder, *drmModeEncoderPtr;

struct drm_mode_create_dumb {
	uint32_t height;
	uint32_t width;
	uint32_t bpp;
	uint32_t flags;
	uint32_t handle;
	uint32_t pitch;
	uint64_t size;
};

struct drm_mode_map_dumb {
	uint32_t handle;
	uint32_t pad;
	uint64_t offset;
};

struct drm_mode_destroy_dumb {
	uint32_t handle;
};

struct drm_mode_fb_cmd {
	uint32_t fb_id;
	uint32_t width;
	uint32_t height;
	uint32_t pitch;
	uint32_t bpp;
	uint32_t depth;
	uint32_t handle;
};

struct drm_get_cap {
	uint64_t capability;
	uint64_t value;
};

struct drm_mode_card_res {
	uint64_t fb_id_ptr;
	uint64_t crtc_id_ptr;
	uint64_t connector_id_ptr;
	uint64_t encoder_id_ptr;
	uint32_t count_fbs;
	uint32_t count_crtcs;
	uint32_t count_connectors;
	uint32_t count_encoders;
	uint32_t min_width;
	uint32_t max_width;
	uint32_t min_height;
	uint32_t max_height;
};

struct drm_mode_get_connector {
	uint64_t encoders_ptr;
	uint64_t modes_ptr;
	uint64_t props_ptr;
	uint64_t prop_values_ptr;
	uint32_t count_modes;
	uint32_t count_props;
	uint32_t count_encoders;
	uint32_t encoder_id;
	uint32_t connector_id;
	uint32_t connector_type;
	uint32_t connector_type_id;
	uint32_t connection;
	uint32_t mm_width;
	uint32_t mm_height;
	uint32_t subpixel;
	uint32_t pad;
};

struct drm_mode_modeinfo {
	uint32_t clock;
	uint16_t hdisplay;
	uint16_t hsync_start;
	uint16_t hsync_end;
	uint16_t htotal;
	uint16_t hskew;
	uint16_t vdisplay;
	uint16_t vsync_start;
	uint16_t vsync_end;
	uint16_t vtotal;
	uint16_t vscan;
	uint32_t vrefresh;
	uint32_t flags;
	uint32_t type;
	char name[32];
};

struct drm_mode_crtc {
	uint64_t set_connectors_ptr;
	uint32_t count_connectors;
	uint32_t crtc_id;
	uint32_t fb_id;
	uint32_t x;
	uint32_t y;
	uint32_t gamma_size;
	uint32_t mode_valid;
	struct drm_mode_modeinfo mode;
};

struct drm_mode_get_encoder {
	uint32_t encoder_id;
	uint32_t encoder_type;
	uint32_t crtc_id;
	uint32_t possible_crtcs;
	uint32_t possible_clones;
};

#define DRM_IOWR(nr, type)				_IOWR('d', nr, type)
#define DRM_IOCTL_MODE_CREATE_DUMB		DRM_IOWR(0xB2, struct drm_mode_create_dumb)
#define DRM_IOCTL_MODE_ADDFB			DRM_IOWR(0xAE, struct drm_mode_fb_cmd)
#define DRM_IOCTL_MODE_DESTROY_DUMB		DRM_IOWR(0xB4, struct drm_mode_destroy_dumb)
#define DRM_IOCTL_MODE_MAP_DUMB			DRM_IOWR(0xB3, struct drm_mode_map_dumb)
#define DRM_IOCTL_MODE_GETCONNECTOR		DRM_IOWR(0xA7, struct drm_mode_get_connector)
#define DRM_IOCTL_MODE_GETCRTC			DRM_IOWR(0xA1, struct drm_mode_crtc)
#define DRM_IOCTL_MODE_SETCRTC			DRM_IOWR(0xA2, struct drm_mode_crtc)
#define DRM_IOCTL_MODE_RMFB				DRM_IOWR(0xAF, unsigned int)
#define DRM_IOCTL_MODE_GETRESOURCES		DRM_IOWR(0xA0, struct drm_mode_card_res)
#define DRM_IOCTL_GET_CAP				DRM_IOWR(0x0c, struct drm_get_cap)
#define DRM_IOCTL_MODE_GETENCODER		DRM_IOWR(0xA6, struct drm_mode_get_encoder)

static int drmIoctl(int fd, unsigned long request, void *arg)
{
	int ret;

	do {
		ret = ioctl(fd, request, arg);
	} while(ret == -1 && (errno == EINTR || errno == EAGAIN));
	return ret;
}

static inline int DRM_IOCTL(int fd, unsigned long cmd, void *arg)
{
	int ret = drmIoctl(fd, cmd, arg);
	return ret < 0 ? -errno : ret;
}

static int drmModeAddFB(int fd, uint32_t width, uint32_t height, uint8_t depth, uint8_t bpp, uint32_t pitch, uint32_t bo_handle, uint32_t * buf_id)
{
	struct drm_mode_fb_cmd f;
	int ret;

	memset(&f, 0, sizeof(f));
	f.width = width;
	f.height = height;
	f.pitch = pitch;
	f.bpp = bpp;
	f.depth = depth;
	f.handle = bo_handle;

	if((ret = DRM_IOCTL(fd, DRM_IOCTL_MODE_ADDFB, &f)))
		return ret;
	*buf_id = f.fb_id;
	return 0;
}

static int drmModeRmFB(int fd, uint32_t bufferId)
{
	return DRM_IOCTL(fd, DRM_IOCTL_MODE_RMFB, &bufferId);
}

static int drmGetCap(int fd, uint64_t capability, uint64_t *value)
{
	struct drm_get_cap cap;
	int ret;

	memset(&cap, 0, sizeof(cap));
	cap.capability = capability;

	ret = drmIoctl(fd, DRM_IOCTL_GET_CAP, &cap);
	if(ret)
		return ret;

	*value = cap.value;
	return 0;
}

static void * drmMalloc(int size)
{
	return calloc(1, size);
}

static void drmFree(void *pt)
{
	free(pt);
}

static void * drmAllocCpy(char * array, int count, int entry_size)
{
	char *r;
	int i;

	if(!count || !array || !entry_size)
		return 0;
	if(!(r = drmMalloc(count * entry_size)))
		return 0;
	for(i = 0; i < count; i++)
		memcpy(r + (entry_size * i), array + (entry_size * i), entry_size);
	return r;
}

static drmModeResPtr drmModeGetResources(int fd)
{
	struct drm_mode_card_res res, counts;
	drmModeResPtr r = 0;

retry:
	memset(&res, 0, sizeof(res));
	if(drmIoctl(fd, DRM_IOCTL_MODE_GETRESOURCES, &res))
		return 0;

	counts = res;
	if(res.count_fbs)
	{
		res.fb_id_ptr = (uint64_t)(unsigned long)(drmMalloc(res.count_fbs * sizeof(uint32_t)));
		if(!res.fb_id_ptr)
			goto err_allocs;
	}
	if(res.count_crtcs)
	{
		res.crtc_id_ptr = (uint64_t)(unsigned long)(drmMalloc(res.count_crtcs * sizeof(uint32_t)));
		if(!res.crtc_id_ptr)
			goto err_allocs;
	}
	if(res.count_connectors)
	{
		res.connector_id_ptr = (uint64_t)(unsigned long)(drmMalloc(res.count_connectors * sizeof(uint32_t)));
		if(!res.connector_id_ptr)
			goto err_allocs;
	}
	if(res.count_encoders)
	{
		res.encoder_id_ptr = (uint64_t)(unsigned long)(drmMalloc(res.count_encoders * sizeof(uint32_t)));
		if(!res.encoder_id_ptr)
			goto err_allocs;
	}

	if(drmIoctl(fd, DRM_IOCTL_MODE_GETRESOURCES, &res))
		goto err_allocs;

	if(counts.count_fbs < res.count_fbs || counts.count_crtcs < res.count_crtcs || counts.count_connectors < res.count_connectors
			|| counts.count_encoders < res.count_encoders)
	{
		drmFree((void *)(unsigned long)(res.fb_id_ptr));
		drmFree((void *)(unsigned long)(res.crtc_id_ptr));
		drmFree((void *)(unsigned long)(res.connector_id_ptr));
		drmFree((void *)(unsigned long)(res.encoder_id_ptr));

		goto retry;
	}

	if(!(r = drmMalloc(sizeof(*r))))
		goto err_allocs;

	r->min_width = res.min_width;
	r->max_width = res.max_width;
	r->min_height = res.min_height;
	r->max_height = res.max_height;
	r->count_fbs = res.count_fbs;
	r->count_crtcs = res.count_crtcs;
	r->count_connectors = res.count_connectors;
	r->count_encoders = res.count_encoders;

	r->fbs = drmAllocCpy((void *)(unsigned long)(res.fb_id_ptr), res.count_fbs, sizeof(uint32_t));
	r->crtcs = drmAllocCpy((void *)(unsigned long)(res.crtc_id_ptr), res.count_crtcs, sizeof(uint32_t));
	r->connectors = drmAllocCpy((void *)(unsigned long)(res.connector_id_ptr), res.count_connectors, sizeof(uint32_t));
	r->encoders = drmAllocCpy((void *)(unsigned long)(res.encoder_id_ptr), res.count_encoders, sizeof(uint32_t));
	if((res.count_fbs && !r->fbs) || (res.count_crtcs && !r->crtcs) || (res.count_connectors && !r->connectors) || (res.count_encoders && !r->encoders))
	{
		drmFree(r->fbs);
		drmFree(r->crtcs);
		drmFree(r->connectors);
		drmFree(r->encoders);
		drmFree(r);
		r = 0;
	}

err_allocs:
	drmFree((void *)(unsigned long)(res.fb_id_ptr));
	drmFree((void *)(unsigned long)(res.crtc_id_ptr));
	drmFree((void *)(unsigned long)(res.connector_id_ptr));
	drmFree((void *)(unsigned long)(res.encoder_id_ptr));

	return r;
}

static void drmModeFreeResources(drmModeResPtr ptr)
{
	if(!ptr)
		return;
	drmFree(ptr->fbs);
	drmFree(ptr->crtcs);
	drmFree(ptr->connectors);
	drmFree(ptr->encoders);
	drmFree(ptr);
}

static drmModeConnectorPtr _drmModeGetConnector(int fd, uint32_t connector_id, int probe)
{
	struct drm_mode_get_connector conn, counts;
	drmModeConnectorPtr r = NULL;
	struct drm_mode_modeinfo stack_mode;

	memset(&conn, 0, sizeof(conn));
	conn.connector_id = connector_id;
	if(!probe)
	{
		conn.count_modes = 1;
		conn.modes_ptr = (uint64_t)(unsigned long)(&stack_mode);
	}
	if(drmIoctl(fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn))
		return 0;
retry:
	counts = conn;
	if(conn.count_props)
	{
		conn.props_ptr = (uint64_t)(unsigned long)(drmMalloc(conn.count_props * sizeof(uint32_t)));
		if(!conn.props_ptr)
			goto err_allocs;
		conn.prop_values_ptr = (uint64_t)(unsigned long)(drmMalloc(conn.count_props * sizeof(uint64_t)));
		if(!conn.prop_values_ptr)
			goto err_allocs;
	}

	if(conn.count_modes)
	{
		conn.modes_ptr = (uint64_t)(unsigned long)(drmMalloc(conn.count_modes * sizeof(struct drm_mode_modeinfo)));
		if(!conn.modes_ptr)
			goto err_allocs;
	}
	else
	{
		conn.count_modes = 1;
		conn.modes_ptr = (uint64_t)(unsigned long)(&stack_mode);
	}

	if(conn.count_encoders)
	{
		conn.encoders_ptr = (uint64_t)(unsigned long)(drmMalloc(conn.count_encoders * sizeof(uint32_t)));
		if(!conn.encoders_ptr)
			goto err_allocs;
	}

	if(drmIoctl(fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn))
		goto err_allocs;

	if(counts.count_props < conn.count_props || counts.count_modes < conn.count_modes || counts.count_encoders < conn.count_encoders)
	{
		drmFree((void *)(unsigned long)(conn.props_ptr));
		drmFree((void *)(unsigned long)(conn.prop_values_ptr));
		if((void *)(unsigned long)(conn.modes_ptr) != &stack_mode)
			drmFree((void *)(unsigned long)(conn.modes_ptr));
		drmFree((void *)(unsigned long)(conn.encoders_ptr));
		goto retry;
	}

	if(!(r = drmMalloc(sizeof(*r))))
	{
		goto err_allocs;
	}

	r->connector_id = conn.connector_id;
	r->encoder_id = conn.encoder_id;
	r->connection = conn.connection;
	r->mmWidth = conn.mm_width;
	r->mmHeight = conn.mm_height;
	r->subpixel = conn.subpixel + 1;
	r->count_modes = conn.count_modes;
	r->count_props = conn.count_props;
	r->props = drmAllocCpy((void *)(unsigned long)(conn.props_ptr), conn.count_props, sizeof(uint32_t));
	r->prop_values = drmAllocCpy((void *)(unsigned long)(conn.prop_values_ptr), conn.count_props, sizeof(uint64_t));
	r->modes = drmAllocCpy((void *)(unsigned long)(conn.modes_ptr), conn.count_modes, sizeof(struct drm_mode_modeinfo));
	r->count_encoders = conn.count_encoders;
	r->encoders = drmAllocCpy((void *)(unsigned long)(conn.encoders_ptr), conn.count_encoders, sizeof(uint32_t));
	r->connector_type = conn.connector_type;
	r->connector_type_id = conn.connector_type_id;

	if((r->count_props && !r->props) || (r->count_props && !r->prop_values) || (r->count_modes && !r->modes) || (r->count_encoders && !r->encoders))
	{
		drmFree(r->props);
		drmFree(r->prop_values);
		drmFree(r->modes);
		drmFree(r->encoders);
		drmFree(r);
		r = 0;
	}

err_allocs:
	drmFree((void *)(unsigned long)(conn.prop_values_ptr));
	drmFree((void *)(unsigned long)(conn.props_ptr));
	if((void *)(unsigned long)(conn.modes_ptr) != &stack_mode)
		drmFree((void *)(unsigned long)(conn.modes_ptr));
	drmFree((void *)(unsigned long)(conn.encoders_ptr));

	return r;
}

static drmModeConnectorPtr drmModeGetConnector(int fd, uint32_t connector_id)
{
	return _drmModeGetConnector(fd, connector_id, 1);
}

static void drmModeFreeConnector(drmModeConnectorPtr ptr)
{
	if(!ptr)
		return;

	drmFree(ptr->encoders);
	drmFree(ptr->prop_values);
	drmFree(ptr->props);
	drmFree(ptr->modes);
	drmFree(ptr);
}

static drmModeCrtcPtr drmModeGetCrtc(int fd, uint32_t crtcId)
{
	struct drm_mode_crtc crtc;
	drmModeCrtcPtr r;

	memset(&crtc, 0, sizeof(crtc));
	crtc.crtc_id = crtcId;

	if(drmIoctl(fd, DRM_IOCTL_MODE_GETCRTC, &crtc))
		return 0;

	if(!(r = drmMalloc(sizeof(*r))))
		return 0;

	r->crtc_id = crtc.crtc_id;
	r->x = crtc.x;
	r->y = crtc.y;
	r->mode_valid = crtc.mode_valid;
	if(r->mode_valid)
	{
		memcpy(&r->mode, &crtc.mode, sizeof(struct drm_mode_modeinfo));
		r->width = crtc.mode.hdisplay;
		r->height = crtc.mode.vdisplay;
	}
	r->buffer_id = crtc.fb_id;
	r->gamma_size = crtc.gamma_size;
	return r;
}

static void drmModeFreeCrtc(drmModeCrtcPtr ptr)
{
	if(!ptr)
		return;
	drmFree(ptr);
}

static drmModeEncoderPtr drmModeGetEncoder(int fd, uint32_t encoder_id)
{
	struct drm_mode_get_encoder enc;
	drmModeEncoderPtr r = NULL;

	memset(&enc, 0, sizeof(enc));
	enc.encoder_id = encoder_id;

	if(drmIoctl(fd, DRM_IOCTL_MODE_GETENCODER, &enc))
		return 0;

	if(!(r = drmMalloc(sizeof(*r))))
		return 0;

	r->encoder_id = enc.encoder_id;
	r->crtc_id = enc.crtc_id;
	r->encoder_type = enc.encoder_type;
	r->possible_crtcs = enc.possible_crtcs;
	r->possible_clones = enc.possible_clones;

	return r;
}

static void drmModeFreeEncoder(drmModeEncoderPtr ptr)
{
	drmFree(ptr);
}

static int drmModeSetCrtc(int fd, uint32_t crtcId, uint32_t bufferId, uint32_t x, uint32_t y, uint32_t * connectors, int count, drmModeModeInfoPtr mode)
{
	struct drm_mode_crtc crtc;

	memset(&crtc, 0, sizeof(crtc));
	crtc.x = x;
	crtc.y = y;
	crtc.crtc_id = crtcId;
	crtc.fb_id = bufferId;
	crtc.set_connectors_ptr = (uint64_t)(unsigned long)(connectors);
	crtc.count_connectors = count;
	if(mode)
	{
		memcpy(&crtc.mode, mode, sizeof(struct drm_mode_modeinfo));
		crtc.mode_valid = 1;
	}
	return DRM_IOCTL(fd, DRM_IOCTL_MODE_SETCRTC, &crtc);
}

struct fb_drm_buf_t {
	uint32_t handle;
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	uint32_t pixlen;
	void * pixels;
	uint32_t fb;
};

struct linux_fb_drm_context_t {
	int fd;
	drmModeRes * res;
	drmModeConnector * conn;
	uint32_t conn_id;
	uint32_t crtc_id;
	uint32_t width;
	uint32_t height;
	uint32_t pwidth;
	uint32_t pheight;
	uint32_t stride;
	uint32_t pixlen;
	int index;
	struct fb_drm_buf_t * drmbuf[2];
	struct linux_dirtylist_t * nl, * ol;
};

static struct fb_drm_buf_t * fb_drm_buf_create(struct linux_fb_drm_context_t * ctx)
{
	struct drm_mode_create_dumb creq;
	struct drm_mode_destroy_dumb dreq;
	struct drm_mode_map_dumb mreq;
	struct fb_drm_buf_t * drmbuf;

	drmbuf = malloc(sizeof(struct fb_drm_buf_t));
	if(!drmbuf)
		return NULL;

	memset(&creq, 0, sizeof(creq));
	creq.width = ctx->width;
	creq.height = ctx->height;
	creq.bpp = 32;
	if(drmIoctl(ctx->fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq) != 0)
	{
		free(drmbuf);
		return NULL;
	}

	drmbuf->handle = creq.handle;
	drmbuf->width = creq.width;
	drmbuf->height = creq.height;
	drmbuf->stride = creq.pitch;
	drmbuf->pixlen = creq.size;
	memset(&dreq, 0, sizeof(dreq));
	dreq.handle = drmbuf->handle;

	if(drmModeAddFB(ctx->fd, drmbuf->width, drmbuf->height, 24, 32, drmbuf->stride, drmbuf->handle, &drmbuf->fb) != 0)
	{
		drmIoctl(ctx->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
		free(drmbuf);
		return NULL;
	}

	memset(&mreq, 0, sizeof(mreq));
	mreq.handle = drmbuf->handle;
	if(drmIoctl(ctx->fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq) != 0)
	{
		drmIoctl(ctx->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
		free(drmbuf);
		return NULL;
	}

	drmbuf->pixels = mmap(0, drmbuf->pixlen, PROT_READ | PROT_WRITE, MAP_SHARED, ctx->fd, mreq.offset);
	if(drmbuf->pixels == MAP_FAILED)
	{
		drmModeRmFB(ctx->fd, drmbuf->fb);
		drmIoctl(ctx->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
		free(drmbuf);
		return NULL;
	}
	memset(drmbuf->pixels, 0, drmbuf->pixlen);
	return drmbuf;
}

static void fb_drm_buf_destroy(struct linux_fb_drm_context_t * ctx, struct fb_drm_buf_t * drmbuf)
{
	struct drm_mode_destroy_dumb dreq;

	if(ctx && drmbuf)
	{
		munmap(drmbuf->pixels, drmbuf->pixlen);
		drmModeRmFB(ctx->fd, drmbuf->fb);
		memset(&dreq, 0, sizeof(dreq));
		dreq.handle = drmbuf->handle;
		drmIoctl(ctx->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
		free(drmbuf);
	}
}

void * linux_fb_drm_open(const char * dev, const char * connector)
{
	struct linux_fb_drm_context_t * ctx;
	uint64_t dumb = 0;
	int type = 0;

	if(connector)
	{
		if(strcasecmp(connector, "vga") == 0)
			type = 1;
		else if(strcasecmp(connector, "dvii") == 0)
			type = 2;
		else if(strcasecmp(connector, "dvid") == 0)
			type = 3;
		else if(strcasecmp(connector, "dvia") == 0)
			type = 4;
		else if(strcasecmp(connector, "composite") == 0)
			type = 5;
		else if(strcasecmp(connector, "svideo") == 0)
			type = 6;
		else if(strcasecmp(connector, "lvds") == 0)
			type = 7;
		else if(strcasecmp(connector, "component") == 0)
			type = 8;
		else if(strcasecmp(connector, "9pindin") == 0)
			type = 9;
		else if(strcasecmp(connector, "displayport") == 0)
			type = 10;
		else if(strcasecmp(connector, "hdmia") == 0)
			type = 11;
		else if(strcasecmp(connector, "hdmib") == 0)
			type = 12;
		else if(strcasecmp(connector, "tv") == 0)
			type = 13;
		else if(strcasecmp(connector, "edp") == 0)
			type = 14;
		else if(strcasecmp(connector, "virtual") == 0)
			type = 15;
		else if(strcasecmp(connector, "dsi") == 0)
			type = 16;
		else if(strcasecmp(connector, "dpi") == 0)
			type = 17;
		else if(strcasecmp(connector, "writeback") == 0)
			type = 18;
		else if(strcasecmp(connector, "spi") == 0)
			type = 19;
		else if(strcasecmp(connector, "usb") == 0)
			type = 20;
		else
			type = 0;
	}

	ctx = malloc(sizeof(struct linux_fb_drm_context_t));
	if(!ctx)
		return NULL;

	ctx->fd = open(dev, O_RDWR | O_CLOEXEC);
	if(ctx->fd < 0)
	{
		free(ctx);
		return NULL;
	}

	if(drmGetCap(ctx->fd, 0x1, &dumb) < 0 || !dumb)
	{
		close(ctx->fd);
		free(ctx);
		return NULL;
	}

	ctx->res = drmModeGetResources(ctx->fd);
	if(!ctx->res)
	{
		close(ctx->fd);
		free(ctx);
		return NULL;
	}

	for(int i = 0; i < ctx->res->count_connectors; i++)
	{
		ctx->conn_id = ctx->res->connectors[i];
		ctx->conn = drmModeGetConnector(ctx->fd, ctx->conn_id);
		if((ctx->conn->connection == DRM_MODE_CONNECTED) && ((type == 0) || (ctx->conn->connector_type == type)))
			break;
		if(ctx->conn)
		{
			drmModeFreeConnector(ctx->conn);
			ctx->conn_id = 0;
			ctx->conn = NULL;
		}
	}

	if(!ctx->conn)
	{
		if(ctx->res)
			drmModeFreeResources(ctx->res);
		close(ctx->fd);
		free(ctx);
		return NULL;
	}

	drmModeEncoder * encoder = drmModeGetEncoder(ctx->fd, ctx->conn->encoder_id);
	if(encoder)
	{
		ctx->crtc_id = encoder->crtc_id;
		drmModeFreeEncoder(encoder);
	}
	else
	{
		for(int i = 0; i < ctx->res->count_crtcs; i++)
		{
			drmModeCrtc * crtc = drmModeGetCrtc(ctx->fd, ctx->res->crtcs[i]);
			if(crtc)
			{
				if(crtc->mode_valid == 0)
				{
					drmModeFreeCrtc(crtc);
					ctx->crtc_id = ctx->res->crtcs[i];
					break;
				}
				drmModeFreeCrtc(crtc);
			}
		}
	}

	ctx->width = ctx->conn->modes[0].hdisplay;
	ctx->height = ctx->conn->modes[0].vdisplay;
	ctx->pwidth = ctx->conn->mmWidth;
	ctx->pheight = ctx->conn->mmHeight;
	ctx->index = 0;
	ctx->drmbuf[0] = fb_drm_buf_create(ctx);
	ctx->drmbuf[1] = fb_drm_buf_create(ctx);
	ctx->nl = linux_dirtylist_alloc(0);
	ctx->ol = linux_dirtylist_alloc(0);
	ctx->stride = ctx->drmbuf[0]->stride;
	ctx->pixlen = ctx->drmbuf[0]->pixlen;

	return ctx;
}

void linux_fb_drm_close(void * context)
{
	struct linux_fb_drm_context_t * ctx = (struct linux_fb_drm_context_t *)context;

	if(ctx)
	{
		if(ctx->conn)
			drmModeFreeConnector(ctx->conn);
		if(ctx->res)
			drmModeFreeResources(ctx->res);
		fb_drm_buf_destroy(ctx, ctx->drmbuf[0]);
		fb_drm_buf_destroy(ctx, ctx->drmbuf[1]);
		linux_dirtylist_free(ctx->nl);
		linux_dirtylist_free(ctx->ol);
		close(ctx->fd);
		free(ctx);
	}
}

int linux_fb_drm_get_width(void * context)
{
	struct linux_fb_drm_context_t * ctx = (struct linux_fb_drm_context_t *)context;
	return ctx->width;
}

int linux_fb_drm_get_height(void * context)
{
	struct linux_fb_drm_context_t * ctx = (struct linux_fb_drm_context_t *)context;
	return ctx->height;
}

int linux_fb_drm_get_pwidth(void * context)
{
	struct linux_fb_drm_context_t * ctx = (struct linux_fb_drm_context_t *)context;
	return ctx->pwidth;
}

int linux_fb_drm_get_pheight(void * context)
{
	struct linux_fb_drm_context_t * ctx = (struct linux_fb_drm_context_t *)context;
	return ctx->pheight;
}

int linux_fb_drm_surface_create(void * context, struct linux_fb_surface_t * surface, int width, int height)
{
	struct linux_fb_drm_context_t * ctx = (struct linux_fb_drm_context_t *)context;
	surface->width = width;
	surface->height = height;
	surface->stride = width << 2;
	surface->pixlen = ctx->pixlen;
	surface->pixels = memalign(4, ctx->pixlen);
	return 1;
}

int linux_fb_drm_surface_destroy(void * context, struct linux_fb_surface_t * surface)
{
	if(surface && surface->pixels)
		free(surface->pixels);
	return 1;
}

int linux_fb_drm_surface_present(void * context, struct linux_fb_surface_t * surface, struct linux_dirtylist_t * l)
{
	struct linux_fb_drm_context_t * ctx = (struct linux_fb_drm_context_t *)context;
	struct linux_dirtylist_t * nl = ctx->nl;
	struct fb_drm_buf_t * drmbuf;
	struct linux_region_t * r;
	unsigned char * p, * q;
	int stride = ctx->stride;
	int offset, line, height;
	int i, j;

	linux_dirtylist_clear(nl);
	linux_dirtylist_merge(nl, ctx->ol);
	linux_dirtylist_merge(nl, l);
	linux_dirtylist_clone(ctx->ol, l);

	ctx->index = (ctx->index + 1) & 0x1;
	drmbuf = ctx->drmbuf[ctx->index];
	if(nl && (nl->count > 0))
	{
		for(i = 0; i < nl->count; i++)
		{
			r = &nl->items[i].region;
			offset = r->y * stride + (r->x << 2);
			line = r->w << 2;
			height = r->h;

			p = (unsigned char *)drmbuf->pixels + offset;
			q = (unsigned char *)surface->pixels + offset;
			for(j = 0; j < height; j++, p += stride, q += stride)
				memcpy(p, q, line);
		}
		drmModeSetCrtc(ctx->fd, ctx->crtc_id, drmbuf->fb, 0, 0, &ctx->conn_id, 1, &ctx->conn->modes[0]);
	}
	return 1;
}

void linux_fb_drm_set_backlight(void * context, int brightness)
{
}

int linux_fb_drm_get_backlight(void * context)
{
	return 0;
}
