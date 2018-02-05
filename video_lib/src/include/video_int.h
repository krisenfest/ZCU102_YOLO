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

#ifndef VIDEO_INT_H
#define VIDEO_INT_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Misc configuration */
#define OUTPUT_PIX_FMT v4l2_fourcc('Y','U','Y','V')
#define INPUT_PIX_FMT  v4l2_fourcc('Y','U','Y','V')

#define POLL_TIMEOUT_MSEC 5000

struct media_device;
struct video_pipeline;
struct vlib_vdev;

#include <glib.h>
#include <video.h>
#include <v4l2_helper.h>

struct stream_handle {
	/* common */
	struct v4l2_dev video_in;		/* input device */
	GQueue *buffer_q_src2filter;
	GQueue *buffer_q_filter2sink;
	GQueue *buffer_q_sink2src;
	struct video_pipeline *vp;

	/* m2m sw stream handle */
	struct video_resolution video_out;
	struct filter_s *fs;
};

/**
 * struct matchtable:
 * @s: String to match compatible items against
 * @init: Init function
 *	  @mte: Match table entry that matched @s.
 *	  @data: Custom data pointer.
 *	  Return: struct vlib_vdev on success,
 *		  NULL for unsupported/invalid input
 */
struct matchtable {
	char *s;
	struct vlib_vdev *(*init)(const struct matchtable *mte, void *data);
};

struct vsrc_ops {
	int (*change_mode)(struct video_pipeline *video_setup,
			   struct vlib_config *config);
	int (*set_media_ctrl)(struct video_pipeline *video_setup,
			      const struct vlib_vdev *vdev);
	int (*streamon)(void);
	int (*streamoff)(void);
	int (*prepare)(struct video_pipeline *vp, const struct vlib_vdev *vdev);
	int (*unprepare)(struct video_pipeline *vp, const struct vlib_vdev *vdev);
};

struct vlib_vdev {
	enum vlib_vsrc_class vsrc_class;
	const char *display_text;
	const char *entity_name;
	union {
		struct {
			struct media_device *mdev;
			int vnode;		/* video node file descriptor */
		} media;
		struct {
			int vnode;		/* video node file descriptor */
		} v4l2;
		struct {
			FILE *fd;
			uint8_t *buf;
			size_t buf_cnt;
			size_t buf_cur;
			const char *filename;
			uint8_t *(*get_frame)(const struct vlib_vdev *vdev,
					      const struct video_pipeline *vp);
		} file;
	} data;
	enum {
		VSRC_TYPE_INVALID,
		VSRC_TYPE_MEDIA,
		VSRC_TYPE_V4L2,
		VSRC_TYPE_FILE,
	} vsrc_type;
	const struct vsrc_ops *ops;
};

static inline int video_src_is_v4l2(const struct vlib_vdev *vdev)
{
	return vdev->vsrc_type == VSRC_TYPE_MEDIA ||
	       vdev->vsrc_type == VSRC_TYPE_V4L2;
}

static inline int video_src_is_file(const struct vlib_vdev *vdev)
{
	return vdev->vsrc_type == VSRC_TYPE_FILE;
}

typedef enum {
	MODE_INIT,
	MODE_CHANGE,
	MODE_EXIT
} app_state;


#include <drm_helper.h>
#include <linux/videodev2.h>

struct levents_counter;

/* global setup for all modes */
struct video_pipeline {
	unsigned int w, h; /* input width, height */
	unsigned int w_out, h_out; /* output width, height */
	unsigned int vtotal, htotal;
	unsigned int stride, stride_out; /* input/output stride */
	unsigned int in_fourcc; /* input pixel format */
	unsigned int out_fourcc; /* output pixel format */
	struct v4l2_fract fps; /* frame rate */
	int app_state;
	struct drm_device drm;
	const struct vlib_vdev *vid_src;
	pthread_t eventloop;
	pthread_t fps_thread;
	int pflip_pending; /* next vblank is considered as pflip_pending=false */
	unsigned int flags;
	int pr_enable; /* partial reconfiguration */
	struct levents_counter *events[NUM_EVENTS];
	int enable_log_event;
	int hdmi_in_width;
	int hdmi_in_height;
	int has_scaler;
	struct filter_tbl *ft;
	size_t buffer_cnt; /* number of frame buffers */
};

int vlib_video_src_init(struct vlib_config_data *cfg);
void vlib_video_src_uninit(void);
struct media_device *vlib_vdev_get_mdev(const struct vlib_vdev *vdev);
void vlib_video_src_class_disable(enum vlib_vsrc_class class);
const char *vlib_video_src_mdev2vdev(struct media_device *media);
int vlib_video_src_get_vnode(const struct vlib_vdev *vsrc);
int vlib_pipeline_v4l2_init(struct stream_handle *sh, struct video_pipeline *s,
			    int mem_type);
size_t vlib_fourcc2bpp(uint32_t fourcc);

void vlib_log(vlib_log_level level, const char *format, ...)
		__attribute__((__format__(__printf__, 2, 3)));
void vlib_log_v(vlib_log_level level, const char *format, va_list args);

#define _vlib_log(level, ...) vlib_log(level, __VA_ARGS__)

#define DEBUG_MODE

#ifdef DEBUG_MODE
#define INFO_MODE
#define WARN_MODE
#define ERROR_MODE
#define vlib_dbg(...) _vlib_log(VLIB_LOG_LEVEL_DEBUG, __VA_ARGS__)
#else
#define vlib_dbg(...) do {} while(0)
#endif

#ifdef INFO_MODE
#define WARN_MODE
#define ERROR_MODE
#define vlib_info(...) _vlib_log(VLIB_LOG_LEVEL_INFO, __VA_ARGS__)
#else
#define vlib_info(...) do {} while(0)
#endif

#ifdef WARN_MODE
#define ERROR_MODE
#define vlib_warn(...) _vlib_log(VLIB_LOG_LEVEL_WARNING, __VA_ARGS__)
#else
#define vlib_warn(...) do {} while(0)
#endif

#ifdef ERROR_MODE
#define vlib_err(...) _vlib_log(VLIB_LOG_LEVEL_ERROR, __VA_ARGS__)
#else
#define vlib_err(...) do {} while(0)
#endif

#ifdef __cplusplus
}
#endif
#endif /* VIDEO_INT_H */
