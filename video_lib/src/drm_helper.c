/******************************************************************************
 * (c) Copyright 2012-2016 Xilinx, Inc. All rights reserved.
 *
 * This file contains confidential and proprietary information of Xilinx, Inc.
 * and is protected under U.S. and international copyright and other
 * intellectual property laws.
 *
 * DISCLAIMER
 * This disclaimer is not a license and does not grant any rights to the
 * materials distributed herewith. Except as otherwise provided in a valid
 * license issued to you by Xilinx, and to the maximum extent permitted by
 * applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
 * FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
 * MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
 * and (2) Xilinx shall not be liable (whether in contract or tort, including
 * negligence, or under any other theory of liability) for any loss or damage
 * of any kind or nature related to, arising under or in connection with these
 * materials, including for any direct, or any indirect, special, incidental,
 * or consequential loss or damage (including loss of data, profits, goodwill,
 * or any type of loss or damage suffered as a result of any action brought by
 * a third party) even if such damage or loss was reasonably foreseeable or
 * Xilinx had been advised of the possibility of the same.
 *
 * CRITICAL APPLICATIONS
 * Xilinx products are not designed or intended to be fail-safe, or for use in
 * any application requiring fail-safe performance, such as life-support or
 * safety devices or systems, Class III medical devices, nuclear facilities,
 * applications related to the deployment of airbags, or any other applications
 * that could lead to death, personal injury, or severe property or
 * environmental damage (individually and collectively, "Critical
 * Applications"). Customer assumes the sole risk and liability of any use of
 * Xilinx products in Critical Applications, subject only to applicable laws
 * and regulations governing limitations on product liability.
 *
 * THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
 * AT ALL TIMES.
 *******************************************************************************/
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/videodev2.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "helper.h"
#include "drm_helper.h"
#include <drm/drm_fourcc.h>
#include "common.h"
#ifdef  WITH_SDSOC
#include <sds_lib.h>
#endif

#define container_of(ptr, type, member) ({ \
    const typeof( ((type *)0)->member ) \
    *__mptr = (ptr);\
    (type *)( (char *)__mptr - offsetof(type,member) );})


/*Parse string format and extract width,height,left,top params */
static inline int parse_rect(char *s, struct v4l2_rect *r)
{
	 return sscanf(s, "%d,%d@%d,%d", &r->width, &r->height,
		 &r->left, &r->top) != 4;
}

static const char __attribute__((__unused__)) *plane_type2str(plane_type type)
{
	switch (type) {
	case PLANE_CURSOR:
		return "cursor";
	case PLANE_OVERLAY:
		return "overlay";
	case PLANE_PRIMARY:
		return "primary";
	default:
		return "invalid/unknown";
	}
}

void drm_buffer_destroy(int fd, struct drm_buffer *b)
{
	struct drm_mode_destroy_dumb destroy_dumb_obj;

#ifdef WITH_SDSOC
	/* Unregister DMABUF with sds framework */
	vlib_dbg("sds_unregister_dmabuf :: %s :: %d\n", __func__, b->dbuf_fd);

	if (sds_unregister_dmabuf((void *)b->drm_buff, b->dbuf_fd)) {
		vlib_err("dmabuf unregistration failed\n");
	}
#endif

	/* unmap drm buffer */
	munmap(b->drm_buff, b->dumb_buff_length);
	close(b->dbuf_fd);

	/* free-up framebuffer */
	drmModeRmFB(fd, b->fb_handle);

	memset(&destroy_dumb_obj, 0, sizeof(destroy_dumb_obj));
	destroy_dumb_obj.handle = b->bo_handle;
	/* destroy dumb buffer */
	drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy_dumb_obj);
}

/*
 * Create dumb buffers and framebuffer for scanout.
 * Requests the DRM subsystem to prepare the buffer for memory-mapping
 */
int drm_buffer_create(struct drm_device *dev, struct drm_buffer *b,
		size_t drm_width, size_t drm_height, size_t drm_stride,
		uint32_t fourcc)
{
	struct drm_mode_create_dumb gem;
	struct drm_mode_map_dumb mreq;
	struct drm_mode_destroy_dumb gem_destroy;
	int ret;

	vlib_dbg("%s :: width:%zu height:%zu stride:%zu\n", __func__,
		 drm_width, drm_height, drm_stride);

	memset(&gem, 0, sizeof(gem));
	gem.width = drm_width;
	gem.height = drm_height;
	gem.bpp = drm_stride / drm_width * 8;

	/*
	 * Creates a gem object.
	 * The kernel will return a 32bit handle that can be used to
	 * manage the buffer with the DRM API
	 */
	ret = ioctl(dev->fd, DRM_IOCTL_MODE_CREATE_DUMB, &gem);
	if (ret) {
		vlib_warn("CREATE_DUMB failed: %s\n", ERRSTR);
		return VLIB_ERROR_INTERNAL;
	}

	b->bo_handle = gem.handle;
	b->dumb_buff_length = gem.size;
	struct drm_prime_handle prime;
	memset(&prime, 0, sizeof(prime));
	prime.handle = b->bo_handle;
	/* Export gem object  to a FD */
	ret = ioctl(dev->fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &prime);
	if (ret) {
		vlib_warn("PRIME_HANDLE_TO_FD failed: %s\n", ERRSTR);
		goto fail_gem;
	}

	b->dbuf_fd = prime.fd;

	uint32_t offsets[4] = { 0 };
	uint32_t pitches[4] = {drm_stride};
	uint32_t bo_handles[4] = {b->bo_handle};

	vlib_dbg("drmModeAddFB2 (args):: %zu %zu %.4s\n", drm_width, drm_height,
		 (const char *)&fourcc);
	/* request the creation of frame buffers */
	ret = drmModeAddFB2(dev->fd, drm_width, drm_height, fourcc, bo_handles,
		pitches, offsets, &b->fb_handle, 0);
	if (ret) {
		vlib_warn("drmModeAddFB2 failed: %s\n", ERRSTR);
		goto fail_prime;
	}

	/* prepare buffer for memory mapping */
	memset(&mreq, 0, sizeof(mreq));
	mreq.handle = b->bo_handle;
	ret = drmIoctl(dev->fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
	if (ret) {
		vlib_err("cannot map dumb buffer (%d): %m\n", errno);
		ret = -errno;
		goto fail_map;
	}

	/* perform actual memory mapping */
	b->drm_buff = mmap(0, gem.size, PROT_READ | PROT_WRITE, MAP_SHARED,
			  dev->fd, mreq.offset);
	if (b->drm_buff == MAP_FAILED) {
		vlib_err("cannot mmap dumb buffer\n");
		ret = -errno;
		goto fail_map;
	}

#ifdef WITH_SDSOC
	/* register buffers with SDx */
	vlib_dbg("sds_register_dmabuf :: %s :: %d\n", __func__, b->dbuf_fd);

	ret = sds_register_dmabuf((void *)b->drm_buff, b->dbuf_fd);
	if (ret) {
		vlib_err("dmabuf registration failed\n");
		goto fail_sds;
	}
#endif

	return VLIB_SUCCESS;

#ifdef WITH_SDSOC
fail_sds:
#endif
	munmap(b->drm_buff, b->dumb_buff_length);
fail_map:
	drmModeRmFB(dev->fd, b->fb_handle);
fail_prime:
	close(b->dbuf_fd);

fail_gem:
	memset(&gem_destroy, 0, sizeof(gem_destroy));
	gem_destroy.handle = b->bo_handle;
	if (ioctl(dev->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &gem_destroy)) {
		vlib_warn("DESTROY_DUMB failed: %s\n", ERRSTR);
		return VLIB_ERROR_INTERNAL;
	}

	return ret;
}

/* Find available CRTC and connector for scanout */
static int drm_find_crtc(struct drm_device *dev)
{
	int ret = -1;

	drmModeRes *res = drmModeGetResources(dev->fd);
	if (!res) {
		vlib_warn("drmModeGetResources failed: %s\n", ERRSTR);
		return VLIB_ERROR_INTERNAL;
	}

	if (res->count_crtcs <= 0) {
		vlib_warn("drm: no crts\n");
		goto done;
	}

	/* Assume first crtc id is ok */
	dev->crtc_index = 0 ;
	dev->crtc_id = res->crtcs[0] ;

	if (res->count_connectors <= 0) {
		vlib_warn("drm: no connectors\n");
		goto done;
	}

	/* Assume first connector is ok */
	dev->con_id = res->connectors[0];

	dev->connector = drmModeGetConnector(dev->fd, dev->con_id);
	if (!dev->connector) {
		vlib_warn("drmModeGetConnector failed: %s\n", ERRSTR);
		goto done;
	}

	ret = VLIB_SUCCESS;

done:
	drmModeFreeResources(res);
	return ret;
}

int drm_try_mode(struct drm_device *dev, int width, int height)
{
	int ret;

	dev->fd = open(dev->dri_card, O_RDWR, 0);
	ASSERT2(dev->fd >= 0, "open DRM device %s failed: %s\n", dev->dri_card,
		ERRSTR);

	ret = drm_find_crtc(dev);
	ASSERT2(!ret, "Failed to find CRTC and/or connector\n");

	drmModeConnector *connector = dev->connector;
	ret = VLIB_ERROR_INVALID_PARAM;

	for (size_t j = 0; j < connector->count_modes; j++) {
		/* Iterate through all the supported modes */
		if (connector->modes[j].hdisplay == width &&
			connector->modes[j].vdisplay == height) {
			ret = VLIB_SUCCESS;
			break;
		}
	}

	drmClose(dev->fd);

	return ret;
}

static int drm_set_mode(struct drm_device *dev, const char *bgnd)
{
	size_t j;
	uint32_t fourcc = 0;
	int ret, mode_found = 0;
	drmModeCrtc *curr_crtc;

	struct video_pipeline *v_pipe = container_of(dev, struct video_pipeline,
						     drm);

	dev->saved_crtc= drmModeGetCrtc(dev->fd, dev->crtc_id);
	ASSERT2(dev->saved_crtc, "Could not get crtc %i\n", dev->crtc_id);

	curr_crtc = dev->saved_crtc;

	drmModeConnector *connector = dev->connector;
	ret = VLIB_SUCCESS;
	for (j = 0; j < connector->count_modes; j++) {
		size_t vrefresh = dev->vrefresh;
		drmModeModeInfoPtr mode = &connector->modes[j];

		/* Iterate through all the supported modes */
		if (mode->hdisplay == v_pipe->w_out &&
		    mode->vdisplay== v_pipe->h_out) {
			if (vrefresh && !(mode->vrefresh == vrefresh)) {
				continue;
			}

			mode_found = 1;
			v_pipe->vtotal = mode->vtotal;
			v_pipe->htotal = mode->htotal;
			dev->fps = mode->vrefresh;
			break;
		}
	}
	/* Assert if requested resolution is not supported by the CRTC display */
	ASSERT2(mode_found,
		"Input Resolution %dx%d not supported by the monitor!\n",
		v_pipe->w_out, v_pipe->h_out);

	drmModePlanePtr plane = drmModeGetPlane(dev->fd,
				dev->prim_plane.drm_plane.plane_id);
	ASSERT2(plane, "failed to get primary plane\n");
	ASSERT2(plane->count_formats, "plane does not support any formats\n");

	/*
	 * find best format for CRTC in the following priority: AR24, RG24, any.
	 * AR24 and RG24 are the 4/3 byte formats used by X11. If none of those
	 * is available use the first format the plane supports.
	 */
	fourcc = plane->formats[0];
	for (size_t i = 0; i < plane->count_formats; i++) {
		if (plane->formats[i] == V4L2_PIX_FMT_ABGR32) {
			fourcc = V4L2_PIX_FMT_ABGR32;
			break;
		}
		if (plane->formats[i] == DRM_FORMAT_RGB888) {
			fourcc = DRM_FORMAT_RGB888;
		}
	}

	drmModeFreePlane(plane);

	ret = drm_buffer_create(dev, &dev->crtc_buf,
				v_pipe->w_out, v_pipe->h_out,
				v_pipe->w_out * vlib_fourcc2bpp(fourcc),
				fourcc);
	ASSERT2(!ret, "failed to create CRTC buffer\n");

	/* get background data */
	if (!bgnd) {
		goto set_crtc;
	}

	FILE *fd = fopen(bgnd, "r");
	if (fd < 0) {
		vlib_warn("unable to open file '%s'\n", bgnd);
		goto set_crtc;
	}

	size_t frame_sz = v_pipe->w_out * v_pipe->h_out * vlib_fourcc2bpp(fourcc);
	ret = fread(dev->crtc_buf.drm_buff, frame_sz, 1, fd);
	if (ret != 1) {
		vlib_warn("failed to read background image\n");
	}

	fclose(fd);

set_crtc:
	/* Set the resolution */
	ret = drmModeSetCrtc(dev->fd, curr_crtc->crtc_id,
			     dev->crtc_buf.fb_handle,
			     0, 0,
			     &dev->con_id, 1, &connector->modes[j]);
	ASSERT2(ret >= 0,
		"drmModeSetCrtc :: Failed Not able to set resolution [%dx%d] on the CRTC: %s\n",
		v_pipe->w_out, v_pipe->h_out, ERRSTR);

	return VLIB_SUCCESS;
}

/* Find DRM preferred mode */
int drm_find_preferred_mode(struct drm_device *dev)
{
	int ret;

	ASSERT2(dev->fd >= 0, "open DRM device %s failed: %s\n", dev->dri_card,
		ERRSTR);

	ret = drm_find_crtc(dev);
	ASSERT2(!ret, "Failed to find CRTC and/or connector\n");

	drmModeConnector *connector = dev->connector;

	if (!connector->count_modes) {
		vlib_warn("connector supports no mode\n");
		return VLIB_ERROR_INTERNAL;
	}

	/* First advertised mode is preferred mode */
	dev->preferred_mode = connector->modes;

	return VLIB_SUCCESS;
}

/* Find an unused plane that supports the requested format */
static int drm_find_plane(struct drm_device *dev, struct vlib_plane *p)
{
	drmModePlaneResPtr planes;
	drmModePlanePtr plane;
	int ret = -1;

	vlib_dbg("%s\n\n", __func__);

	planes = drmModeGetPlaneResources(dev->fd);
	if (!planes) {
		vlib_warn("drmModeGetPlaneResources failed: %s\n", ERRSTR);
		return VLIB_ERROR_INTERNAL;
	}

	for (size_t i = 0; i < planes->count_planes; ++i) {
		size_t j;

		plane = drmModeGetPlane(dev->fd, planes->planes[i]);
		if (!planes) {
			vlib_warn("drmModeGetPlane failed: %s\n", ERRSTR);
			break;
		}

		/* Retrieve plane type - PRIMARY, OVERLAY or CURSOR */
		plane_type type = drm_get_plane_type(dev, plane->plane_id);
		if (type == PLANE_PRIMARY) {
			dev->prim_plane.drm_plane = *plane;

			if (dev->overlay_plane.drm_plane.plane_id) {
				break;
			}
		}

		vlib_dbg("plane %zu/%d:\n", i + 1, planes->count_planes);
		vlib_dbg("\tcrtc id: %d\n", plane->crtc_id);
		vlib_dbg("\tplane id: %d\n", plane->plane_id);
		vlib_dbg("\tplane format: %d\n", plane->formats[0]);
		vlib_dbg("\tplane type: %s\n\n", plane_type2str(type));

		if (dev->overlay_plane.drm_plane.plane_id) {
			continue;
		}

		if (!(plane->possible_crtcs & (1 << dev->crtc_index))) {
			drmModeFreePlane(plane);
			continue;
		}

		if (p->id && p->id != plane->plane_id) {
			drmModeFreePlane(plane);
			continue;
		}

		for (j = 0; j < plane->count_formats; ++j) {
			if (plane->formats[j] == dev->format)
				break;
		}

		if (j == plane->count_formats) {
			drmModeFreePlane(plane);
			continue;
		}

		dev->overlay_plane.drm_plane = *plane;
		drmModeFreePlane(plane);

		ret = VLIB_SUCCESS;

		if (dev->prim_plane.drm_plane.plane_id) {
			break;
		}
	}

	drmModeFreePlaneResources(planes);
	return ret;
}

/* Initialize DRM module query CRTC/Plane configuration*/
void drm_init(struct drm_device *dev, struct vlib_plane *plane)
{
	int ret;

	dev->fd = open(dev->dri_card, O_RDWR, 0);
	ASSERT2(dev->fd >= 0, "open DRM device %s failed: %s\n", dev->dri_card,
		ERRSTR);

	drmSetVersion sv;
	memset(&sv, 0, sizeof(sv));
	sv.drm_di_major = 1;
	sv.drm_di_minor = 4;
	sv.drm_dd_major = -1;
	sv.drm_dd_minor = -1;
	ret = drmSetInterfaceVersion(dev->fd, &sv);
	ASSERT2(!ret, "failed to set DRM interface version\n");

	ret = drm_find_crtc(dev);
	ASSERT2(!ret, "failed to find CRTC and/or connector\n");

	/* enable universal plane */
	ret = drmSetClientCap(dev->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
	ASSERT2(!ret, "universal plane not supported\n");

	ret = drm_find_plane(dev, plane);
	ASSERT2(!ret, "failed to find compatible plane\n");
}

/* Allocate frame-buffer for display, creates user-space mapping and set CRTC mode*/
void drm_post_init(struct drm_device *dev, const char *bgnd)
{
	int ret;
	struct video_pipeline *v_pipe;

	v_pipe = container_of(dev, struct video_pipeline, drm);

	for (size_t i = 0; i < dev->buffer_cnt; ++i) {
		ret = drm_buffer_create(dev, &dev->d_buff[i],
					dev->overlay_plane.vlib_plane.width,
					dev->overlay_plane.vlib_plane.height,
					v_pipe->stride_out, dev->format);
		dev->d_buff[i].index=i;
		ASSERT2(!ret, "failed to create buffer%zu\n", i);
	}
	vlib_dbg("buffers ready\n");

	/* Startup DRM settings */
	if (v_pipe->app_state == MODE_CHANGE || v_pipe->app_state == MODE_INIT)
		drm_set_mode(dev, bgnd);
}

/* Un-initialize drm module , freeup allocated resources */
void drm_uninit (struct drm_device *dev)
{
	drmModePlaneResPtr planes;
	drmModePlanePtr plane;

	planes = drmModeGetPlaneResources(dev->fd);
	if (!planes) {
		vlib_warn("drmModeGetPlaneResources failed: %s\n", ERRSTR);
		return;
	}

	for (size_t i = 0; i < planes->count_planes; ++i) {
		plane = drmModeGetPlane(dev->fd, planes->planes[i]);
		if (!planes) {
			vlib_warn("drmModeGetPlane failed: %s\n", ERRSTR);
			break;
		}
		drmModeFreePlane(plane);
	}

	/* delete dumb buffers */
	for (size_t i = 0; i < dev->buffer_cnt; i++) {
		drm_buffer_destroy(dev->fd, &dev->d_buff[i]);
	}

	drm_buffer_destroy(dev->fd, &dev->crtc_buf);

	/* restore saved CRTC configuration */
	drmModeSetCrtc(dev->fd, dev->saved_crtc->crtc_id,
				dev->saved_crtc->buffer_id,
				dev->saved_crtc->x,
				dev->saved_crtc->y,
				&dev->con_id,
				1,
				&dev->saved_crtc->mode);
	drmModeFreeCrtc(dev->saved_crtc);
	drmDropMaster(dev->fd);
	close(dev->fd);
	free(dev->d_buff);
}

/* Configures plane with buffer index to be selected for next scanout */
int drm_set_plane(struct drm_device *dev, int index)
{
	/*
	 * Configure plane, the crtc then blends the content from the
	 * plane over the CRTC framebuffer buffer during scanout
	 */
	return drmModeSetPlane(dev->fd, dev->overlay_plane.drm_plane.plane_id,
				dev->crtc_id, dev->d_buff[index].fb_handle, 0,
				dev->overlay_plane.vlib_plane.xoffs, /* crtx_x */
				dev->overlay_plane.vlib_plane.yoffs, /* crtc_y */
				dev->overlay_plane.vlib_plane.width, /* crtc_w */
				dev->overlay_plane.vlib_plane.height, /* crtc_h */
				0, 0, /* src_x, src_y */
				dev->overlay_plane.vlib_plane.width << 16, /* src_w */
				dev->overlay_plane.vlib_plane.height << 16); /* src_h */
}

int drm_wait_vblank(struct drm_device *dev, void *d_ptr)
{
	int ret;
	drmVBlank vblank;
	vblank.request.type = DRM_VBLANK_EVENT | DRM_VBLANK_RELATIVE;
	vblank.request.sequence = 1;
	vblank.request.signal = (unsigned long)d_ptr;
	ret = drmWaitVBlank(dev->fd, &vblank);
	ASSERT2(!ret, "drmWaitVBlank failed: %s\n", ERRSTR);
	return VLIB_SUCCESS;
}

/* Set DRM plane property for input property name and value */
int drm_set_plane_prop(struct drm_device *dev, unsigned int plane_id, const char *prop_name, int prop_val)
{
	drmModeObjectPropertiesPtr props;
	int ret = -1;

	props = drmModeObjectGetProperties(dev->fd, plane_id, DRM_MODE_OBJECT_PLANE);
	if (!props) {
		return ret;
	}

	for (size_t i = 0; i < props->count_props; i++) {
		drmModePropertyPtr prop = drmModeGetProperty(dev->fd, props->props[i]);

		if (!strcmp(prop->name, prop_name)) {
			ret = drmModeObjectSetProperty(dev->fd, plane_id,
						       DRM_MODE_OBJECT_PLANE,
						       prop->prop_id,
						       prop_val);
			drmModeFreeProperty(prop);
			break;
		}
		drmModeFreeProperty(prop);
	}
	drmModeFreeObjectProperties(props);

	return ret;
}

plane_type drm_get_plane_type(struct drm_device *dev, unsigned int plane_id)
{
	drmModeObjectPropertiesPtr props;
	plane_type type = PLANE_NONE;
	int found = 0;

	props = drmModeObjectGetProperties(dev->fd, plane_id,
					   DRM_MODE_OBJECT_PLANE);
	ASSERT2(props, "DRM get_properties failed\n");

	for (size_t i = 0; i < props->count_props && !found; i++) {
		drmModePropertyPtr prop;
		const char *enum_name = NULL;

		prop = drmModeGetProperty(dev->fd, props->props[i]);
		ASSERT2(prop, "DRM get_property failed\n");

		if (strcmp(prop->name, "type") == 0) {
			for (size_t j = 0; j < prop->count_enums; j++) {
				if (prop->enums[j].value ==
				    props->prop_values[i]) {
					enum_name = prop->enums[j].name;
					break;
				}
			}

			if (strcmp(enum_name, "Primary") == 0)
				type = PLANE_PRIMARY;
			else if (strcmp(enum_name, "Overlay") == 0)
				type = PLANE_OVERLAY;
			else if (strcmp(enum_name, "Cursor") == 0)
				type = PLANE_CURSOR;
			else if (!found)
				vlib_warn("Invalid DRM Plane type\n");

			found = 1;
		}

		drmModeFreeProperty(prop);
	}

	ASSERT2(found, "Invalid DRM Plane type\n");
	drmModeFreeObjectProperties(props);

	return type;
}

int drm_set_plane_state(struct drm_device *dev, unsigned int plane_id, int enable)
{
	int fb_id = 0, flags = 0;
	drmModePlanePtr plane = drmModeGetPlane(dev->fd, plane_id);

	/* If plane is to be enabled restore original frame-buffer id
	    to disable it set it to NULL*/
	if (enable)
		fb_id = plane->fb_id;

	vlib_dbg("%s :: %d %d %d %d %d %d\n", __func__,
		 plane->plane_id,dev->crtc_id,plane->crtc_x, plane->crtc_y,
		 plane->x, plane->y);
	/* note src coords (last 4 args) are in Q16 format */
	drmModeSetPlane(dev->fd, plane->plane_id, dev->crtc_id, fb_id, flags,
			dev->overlay_plane.vlib_plane.xoffs, /* crtx_x */
			dev->overlay_plane.vlib_plane.yoffs, /* crtc_y */
			dev->overlay_plane.vlib_plane.width, /* crtc_w */
			dev->overlay_plane.vlib_plane.height, /* crtc_h */
			0, 0, /* src_x, src_y */
			dev->overlay_plane.vlib_plane.width << 16, /* src_w */
			dev->overlay_plane.vlib_plane.height << 16); /* src_h */

	drmModeFreePlane(plane);

	return VLIB_SUCCESS;
}

/* Set primary plane offset (x,y) */
/* TODO: Make it generic for all planes */
int drm_set_prim_plane_pos(struct drm_device *dev, int x, int y)
{
	int flags = 0;
	drmModePlanePtr prim_plane = &(dev->prim_plane.drm_plane);

	struct video_pipeline *v_pipe;
	v_pipe = container_of(dev, struct video_pipeline, drm);

	prim_plane->crtc_x = x;
	prim_plane->crtc_y = y;

	vlib_dbg("%s :: %d %d %d %d %d %d %d\n", __func__,
		 prim_plane->plane_id,dev->crtc_id, prim_plane->fb_id,
		 prim_plane->crtc_x, prim_plane->crtc_y, prim_plane->x,
		 prim_plane->y);

	/* note src coords (last 4 args) are in Q16 format */
	drmModeSetPlane(dev->fd, prim_plane->plane_id, dev->crtc_id,
			prim_plane->fb_id, flags, prim_plane->crtc_x,
			prim_plane->crtc_y, v_pipe->w_out,
			v_pipe->h_out - prim_plane->crtc_y, 0, 0, v_pipe->w_out << 16,
			(v_pipe->h_out - prim_plane->crtc_y) << 16);

	drmModeFreePlane(prim_plane);

	return VLIB_SUCCESS;
}
