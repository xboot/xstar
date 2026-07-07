#ifndef __XSTAR_DRIVER_CAMERA_FRAME_H__
#define __XSTAR_DRIVER_CAMERA_FRAME_H__

#ifdef __cplusplus
extern "C" {
#endif

enum camera_format_t {
	CAMERA_FORMAT_ARGB	= 0,	/* packed argb 32bits */
	CAMERA_FORMAT_YUYV	= 1,	/* yuyv 4:2:2 packed */
	CAMERA_FORMAT_UYVY	= 2,	/* uyvy 4:2:2 packed */
	CAMERA_FORMAT_NV12	= 3,	/* yuv 4:2:0, with one y plane and one packed u + v */
	CAMERA_FORMAT_NV21	= 4,	/* yuv 4:2:0, with one y plane and one packed v + u */
	CAMERA_FORMAT_YU12	= 5,	/* yuv 4:2:0 planar */
	CAMERA_FORMAT_YV12	= 6,	/* yvu 4:2:0 planar */
	CAMERA_FORMAT_MJPG	= 7,	/* motion jpeg */
};

struct camera_frame_t {
	enum camera_format_t fmt;
	int width;
	int height;
	int buflen;
	void * buf;
};

void camera_frame_to_argb(struct camera_frame_t * frame, void * pixels);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_CAMERA_FRAME_H__ */
