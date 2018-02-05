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
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <drm/drm_fourcc.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "platform.h"
#include "common.h"
#include "helper.h"
#include "gpio_utils.h"
#include "video_int.h"
#include "log_events.h"
#include "m2m_sw_pipeline.h"
#include "mediactl_helper.h"
#include "s2m_pipeline.h"
#include "filter.h"

/* Maximum number of bytes in a log line */
#define VLIB_LOG_SIZE 256

/* number of frame buffers */
#define BUFFER_CNT_MIN     6
#define BUFFER_CNT_DEFAULT 6

#define DRI_CARD_DP	0
#define DRI_CARD_HDMI	1

/*
 * We control the platform and can hence hardcode the path to the UIO
 * node. Nevertheless, it would be more elegant to discover the correct
 * node and parameters from /sys/class/uio/
 */
#define DDR_QOS_MAP_FILE	"/dev/uio0"
#define AFIFM_HP0_MAP_FILE	"/dev/uio3"
#define AFIFM_HP1_MAP_FILE	"/dev/uio4"
#define AFIFM_HP3_MAP_FILE	"/dev/uio6"

/* global variables */
char vlib_errstr[VLIB_ERRSTR_SIZE];

static struct video_pipeline *video_setup;

struct register_data {
	uintptr_t offs;
	uint32_t val;
};

#define DDR_QOS_PORT_TYPE			0
#define DDR_QOS_PORT_TYPE_BEST_EFFORT		0
#define DDR_QOS_PORT_TYPE_LOW_LATENCY		1
#define DDR_QOS_PORT_TYPE_VIDEO			2
#define DDR_QOS_PORT_TYPE_PORT0_TYPE_SHIFT	0
#define DDR_QOS_PORT_TYPE_PORT1R_TYPE_SHIFT	2
#define DDR_QOS_PORT_TYPE_PORT2R_TYPE_SHIFT	6
#define DDR_QOS_PORT_TYPE_PORT3_TYPE_SHIFT	10
#define DDR_QOS_PORT_TYPE_PORT4_TYPE_SHIFT	12
#define DDR_QOS_PORT_TYPE_PORT5_TYPE_SHIFT	14
static const struct register_data qos_data_dp_ddr[] = {
	{ .offs = DDR_QOS_PORT_TYPE,
	  .val = DDR_QOS_PORT_TYPE_LOW_LATENCY << DDR_QOS_PORT_TYPE_PORT0_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_LOW_LATENCY << DDR_QOS_PORT_TYPE_PORT1R_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_LOW_LATENCY << DDR_QOS_PORT_TYPE_PORT2R_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_VIDEO << DDR_QOS_PORT_TYPE_PORT3_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_BEST_EFFORT << DDR_QOS_PORT_TYPE_PORT4_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_BEST_EFFORT << DDR_QOS_PORT_TYPE_PORT5_TYPE_SHIFT, },
};

static const struct register_data qos_data_hdmi_ddr[] = {
	{ .offs = DDR_QOS_PORT_TYPE,
	  .val = DDR_QOS_PORT_TYPE_LOW_LATENCY << DDR_QOS_PORT_TYPE_PORT0_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_LOW_LATENCY << DDR_QOS_PORT_TYPE_PORT1R_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_LOW_LATENCY << DDR_QOS_PORT_TYPE_PORT2R_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_BEST_EFFORT << DDR_QOS_PORT_TYPE_PORT3_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_BEST_EFFORT << DDR_QOS_PORT_TYPE_PORT4_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_BEST_EFFORT << DDR_QOS_PORT_TYPE_PORT5_TYPE_SHIFT, },
};

#define AFIFM_RDQOS	8
#define AFIFM_WRQOS	0x1c
static const struct register_data qos_data_afifmhp1[] = {
	{ .offs = AFIFM_WRQOS, .val = 0, },
};

static const struct register_data qos_data_afifmhp3[] = {
	{ .offs = AFIFM_RDQOS, .val = 0, },
	{ .offs = AFIFM_WRQOS, .val = 0, },
};

static const struct register_data qos_data_hdmi_afifmhp0[] = {
	{ .offs = AFIFM_RDQOS, .val = 0xf, },
};

struct register_init_data {
	const char *fn;
	size_t sz;
	const struct register_data *data;
};

#define DEFINE_RINIT_DATA(_fn, _arry)	{ .fn = _fn, .sz = ARRAY_SIZE(_arry), .data = _arry, }
static const struct register_init_data qos_settings_dp[] = {
	DEFINE_RINIT_DATA(DDR_QOS_MAP_FILE, qos_data_dp_ddr),
	DEFINE_RINIT_DATA(AFIFM_HP1_MAP_FILE, qos_data_afifmhp1),
	DEFINE_RINIT_DATA(AFIFM_HP3_MAP_FILE, qos_data_afifmhp3),
};

static const struct register_init_data qos_settings_hdmi[] = {
	DEFINE_RINIT_DATA(DDR_QOS_MAP_FILE, qos_data_hdmi_ddr),
	DEFINE_RINIT_DATA(AFIFM_HP0_MAP_FILE, qos_data_hdmi_afifmhp0),
	DEFINE_RINIT_DATA(AFIFM_HP1_MAP_FILE, qos_data_afifmhp1),
	DEFINE_RINIT_DATA(AFIFM_HP3_MAP_FILE, qos_data_afifmhp3),
};

struct register_init {
	const struct register_init_data *data;
	size_t sz;
};

static const struct register_init qos_settings[] = {
	{ .data = qos_settings_dp, .sz = ARRAY_SIZE(qos_settings_dp) },
	{ .data = qos_settings_hdmi, .sz = ARRAY_SIZE(qos_settings_hdmi) },
};

static int vlib_platform_set_qos(size_t qos_setting)
{
	int ret = 0;

	const struct register_init *r = &qos_settings[qos_setting];
	ASSERT2(r, "invalid device\n");

	for (size_t i = 0; i < r->sz; i++) {
		const struct register_init_data *rb = &r->data[i];

		int fd = open(rb->fn, O_RDWR);
		if (fd == -1) {
			vlib_err("%s: failed to open file '%s'\n", strerror(errno),
				 rb->fn);
			continue;
		}

		uint32_t *map = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (map == MAP_FAILED) {
			vlib_err("%s: failed to mmap file '%s'\n", strerror(errno),
				 rb->fn);
			ret = VLIB_ERROR_FILE_IO;
			goto err_close;
		}

		for (size_t j = 0; j < rb->sz; j++) {
			const struct register_data *rd = &rb->data[j];

			map[rd->offs / sizeof(uint32_t)] = rd->val;
		}

		munmap(map, 0x1000);
err_close:
		close(fd);
	}

	return ret;
}

static int vlib_platform_clk_mux_setup(unsigned int device)
{
	int ret = gpio_export(GPIO_PS_CLK_MUX);
	if (ret) {
		vlib_warn("failed to export GPIO %u\n", GPIO_PS_CLK_MUX);
	}

	ret = gpio_dir_out(GPIO_PS_CLK_MUX);
	if (ret) {
		vlib_err("failed to set GPIO %u direction\n", GPIO_PS_CLK_MUX);
		gpio_unexport(GPIO_PS_CLK_MUX);
		return ret;
	}

	ret = gpio_value(GPIO_PS_CLK_MUX, !!device);
	if (ret) {
		vlib_err("failed to set GPIO %u value\n", GPIO_PS_CLK_MUX);
		gpio_unexport(GPIO_PS_CLK_MUX);
		return ret;
	}

	return 0;
}

static void vlib_platform_clk_mux_cleanup()
{
	gpio_unexport(GPIO_PS_CLK_MUX);
}

static int vlib_platform_setup(struct vlib_config_data *cfg)
{
	int ret = 0;

	switch (cfg->dri_card_id) {
	case DRI_CARD_DP:
	case DRI_CARD_HDMI:
		ret = vlib_platform_set_qos(cfg->dri_card_id);
		if (ret) {
			return ret;
		}
		break;
	default:
		return VLIB_ERROR_INVALID_PARAM;
		break;
	}

	return ret;
}

static void vlib_platform_cleanup(struct video_pipeline *vp)
{
	vlib_platform_clk_mux_cleanup();
}

static int vlib_filter_init(struct video_pipeline *vp)
{
	int ret;
	struct filter_tbl *ft = vp->ft;
	struct filter_s *fs;
	struct filter_init_data fid = {
		.in_width = vp->w,
		.in_height = vp->h,
		.in_fourcc = vp->in_fourcc,
		.out_width = vp->drm.overlay_plane.vlib_plane.width,
		.out_height = vp->drm.overlay_plane.vlib_plane.height,
		.out_fourcc = vp->out_fourcc,
	};

	for (size_t i = 0; i < ft->size; i++) {
		fs = g_ptr_array_index(ft->filter_types, i);
		if (!fs) {
			return VLIB_ERROR_OTHER;
		}

		/* Initialize filter */
		ret = fs->ops->init(fs, &fid);
		if (ret) {
			vlib_warn("initializing filter '%s' failed\n",
				  filter_type_get_display_text(fs));
			filter_type_unregister(ft, fs);
			i--;
			continue;
		}

		/* Prefetch partial binaries of filters */
		if (video_setup->flags & VLIB_CFG_FLAG_PR_ENABLE) {
			filter_type_prefetch_bin(fs);
		}
	}

	return VLIB_SUCCESS;
}

int vlib_drm_try_mode(unsigned int dri_card_id, int width, int height)
{
	struct drm_device drm_dev;

	snprintf(drm_dev.dri_card, sizeof(drm_dev.dri_card),
		 "/dev/dri/card%u", dri_card_id);

	return drm_try_mode(&drm_dev, width, height);
}

static int vlib_drm_init(struct vlib_config_data *cfg)
{
	size_t bpp;
	struct drm_device *drm_dev = &video_setup->drm;

	snprintf(drm_dev->dri_card, sizeof(drm_dev->dri_card),
		 "/dev/dri/card%u", cfg->dri_card_id);
	drm_dev->overlay_plane.vlib_plane = cfg->plane;
	drm_dev->format = video_setup->out_fourcc;
	drm_dev->vrefresh = cfg->vrefresh;
	drm_dev->buffer_cnt = cfg->buffer_cnt;

	drm_dev->d_buff = calloc(drm_dev->buffer_cnt, sizeof(*drm_dev->d_buff));
	ASSERT2(drm_dev->d_buff, "failed to allocate DRM buffer structs\n");

	bpp = vlib_fourcc2bpp(drm_dev->format);
	if (!bpp) {
		vlib_err("unsupported pixel format '%s'\n",
			 (const char *)&drm_dev->format);
		return VLIB_ERROR_INVALID_PARAM;
	}

	drm_init(drm_dev, &cfg->plane);

	/* Set display resolution */
	if (!cfg->height_out) {
		/* set preferred mode */
		int ret = drm_find_preferred_mode(drm_dev);
		if (ret)
			return ret;

		video_setup->h_out = drm_dev->preferred_mode->vdisplay;
		video_setup->w_out = drm_dev->preferred_mode->hdisplay;

		if (!video_setup->h) {
			video_setup->h = video_setup->h_out;
			video_setup->w = video_setup->w_out;
			video_setup->stride = video_setup->w * bpp;
		}
	} else {
		video_setup->h_out = cfg->height_out;
		video_setup->w_out = cfg->width_out;
	}

	/* if not specified on the command line make the plane fill the whole screen */
	if (!cfg->plane.width) {
		drm_dev->overlay_plane.vlib_plane.width = video_setup->w_out;
		drm_dev->overlay_plane.vlib_plane.height = video_setup->h_out;
	}

	video_setup->stride_out = drm_dev->overlay_plane.vlib_plane.width * bpp;

	drm_post_init(drm_dev, cfg->drm_background);

	if (!(cfg->flags & VLIB_CFG_FLAG_MULTI_INSTANCE)) {
		/* Move video layer to the back and disable global alpha */
		if (drm_set_plane_prop(drm_dev,
				       video_setup->drm.overlay_plane.drm_plane.plane_id,
				       "zpos", 0)) {
			vlib_warn("failed to set zpos\n");
		}

		if (drm_set_plane_prop(drm_dev,
				       video_setup->drm.prim_plane.drm_plane.plane_id,
				       "zpos", 1) ) {
			vlib_warn("failed to set zpos\n");
		}

		if (drm_set_plane_prop(drm_dev,
				       video_setup->drm.prim_plane.drm_plane.plane_id,
				       "global alpha enable", 0) ) {
			vlib_warn("failed to set 'global alpha'\n");
		}
	}

	vlib_dbg("vlib :: DRM Init done ..\n");

	return VLIB_SUCCESS;
}

int vlib_init(struct vlib_config_data *cfg)
{
	int ret;
	size_t bpp;

	ret = vlib_platform_setup(cfg);
	if (ret) {
		return ret;
	}

	cfg->buffer_cnt = cfg->buffer_cnt ? cfg->buffer_cnt : BUFFER_CNT_DEFAULT;
	if (cfg->buffer_cnt < BUFFER_CNT_MIN) {
		vlib_warn("buffer-count = %zu too low, using %u\n",
			  cfg->buffer_cnt, BUFFER_CNT_MIN);
		cfg->buffer_cnt = BUFFER_CNT_MIN;
	}

	ret = vlib_video_src_init(cfg);
	if (ret)
		return ret;

	/* set clock mux if TPG is present */
	for (size_t i = 0; i < vlib_video_src_cnt_get(); i++) {
		if (vlib_video_src_get_class(vlib_video_src_get(i)) ==
		    VLIB_VCLASS_TPG) {
			ret = vlib_platform_clk_mux_setup(cfg->dri_card_id);
			if (ret) {
				vlib_warn("failed to set TPG clock mux\n");
			}
			break;
		}
	}

	/* Allocate video_setup struct and zero out memory */
	video_setup = calloc (1, sizeof(*video_setup));
	video_setup->app_state = MODE_INIT;
	video_setup->in_fourcc = cfg->fmt_in ? cfg->fmt_in : INPUT_PIX_FMT;
	video_setup->out_fourcc = cfg->fmt_out ? cfg->fmt_out : OUTPUT_PIX_FMT;
	video_setup->flags = cfg->flags;
	video_setup->ft = cfg->ft;
	video_setup->buffer_cnt = cfg->buffer_cnt;
	for (size_t i = 0; i < NUM_EVENTS; i++) {
		const char *event_name[] = {
			"Capture", "Display", "Filter-In", "Filter-Out",
		};
		video_setup->events[i] = levents_counter_create(event_name[i]);
		ASSERT2(video_setup->events[i], "failed to create event counter\n");
	}

	bpp = vlib_fourcc2bpp(video_setup->in_fourcc);
	if (!bpp) {
		vlib_err("unsupported pixel format '%.4s'\n",
			 (const char *)&video_setup->in_fourcc);
		return VLIB_ERROR_INVALID_PARAM;
	}

	/* Set input resolution */
	video_setup->h = cfg->height_in;
	video_setup->w = cfg->width_in;
	video_setup->stride = video_setup->w * vlib_fourcc2bpp(video_setup->in_fourcc);
	video_setup->fps.numerator = cfg->fps.numerator;
	video_setup->fps.denominator = cfg->fps.denominator;

	/* Scaler present in design */
	video_setup->has_scaler = VCAP_HDMI_HAS_SCALER;

	ret = vlib_drm_init(cfg);
	if (ret) {
		return ret;
	}

	/* Initialize filters */
	if (video_setup->ft) {
		ret = vlib_filter_init(video_setup);
		if (ret) {
			return ret;
		}
	}

	/* disable TPG unless input and output resolution match */
	if (video_setup->w != video_setup->w_out ||
		video_setup->h != video_setup->h_out) {
		vlib_video_src_class_disable(VLIB_VCLASS_TPG);
	}

	return ret;
}

int vlib_get_active_height(void)
{
	return video_setup->h_out;
}

int vlib_get_active_width(void)
{
	return video_setup->w_out;
}

static int vlib_pipeline_term_threads(struct video_pipeline *vp)
{
	int ret = 0;

	int ret_i = pthread_cancel(video_setup->fps_thread);
	if (ret_i) {
		vlib_warn("failed to cancel fps thread (%d)\n",ret);
		ret |= ret_i;
	}
	ret_i = pthread_join(video_setup->fps_thread, NULL);
	if (ret_i) {
		vlib_warn("failed to join fps thread (%d)\n",ret);
		ret |= ret_i;
	}
	ret_i = pthread_cancel(video_setup->eventloop);
	if (ret_i) {
		vlib_warn("failed to cancel eventloop(%d)\n",ret);
		ret |= ret_i;
	}

	ret_i = pthread_join(video_setup->eventloop, NULL);
	if (ret_i) {
		vlib_warn("failed to join eventloop (%d)\n",ret);
		ret |= ret_i;
	}

	video_setup->eventloop = 0;

	return ret;
}

int vlib_pipeline_stop(void)
{
	int ret = 0;

	/* Add cleanup code */
	if (video_setup->eventloop) {
		/* Set application state */
		video_setup->app_state = MODE_EXIT;
		/* Stop previous running mode if any */
		ret |= vlib_pipeline_term_threads(video_setup);
	}
	if (!(video_setup->flags & VLIB_CFG_FLAG_MULTI_INSTANCE)) {
		/* Disable video layer on pipeline stop */
		ret |= drm_set_plane_state(&video_setup->drm, video_setup->drm.overlay_plane.drm_plane.plane_id, 0);
	}

	return ret;
}

static int vlib_filter_uninit(struct filter_s *fs)
{
	if (!fs)
		return VLIB_ERROR_OTHER;

	/* free buffers for partial bitstreams */
	if (video_setup->flags & VLIB_CFG_FLAG_PR_ENABLE)
		filter_type_free_bin(fs);

	return VLIB_SUCCESS;
}

int vlib_uninit(void)
{
	struct filter_s *fs;
	int ret;

	drm_uninit(&video_setup->drm);

	vlib_video_src_uninit();

	/* Uninitialize filters */
	if (video_setup->ft) {
		for (size_t i = 0; i < video_setup->ft->size; i++) {
			fs = filter_type_get_obj(video_setup->ft, i);
			ret = vlib_filter_uninit(fs);
			if (ret)
				return ret;
		}
		g_ptr_array_free(video_setup->ft->filter_types, TRUE);
	}

	for (size_t i = 0; i < NUM_EVENTS; i++) {
		levents_counter_destroy(video_setup->events[i]);
	}

	vlib_platform_cleanup(video_setup);

	free(video_setup);

	return ret;
}

static void drm_event_handler(int fd __attribute__((__unused__)),
	unsigned int frame __attribute__((__unused__)),
	unsigned int sec __attribute__((__unused__)),
	unsigned int usec __attribute__((__unused__)),
	void *data )
{
	struct video_pipeline *v_pipe = (struct video_pipeline *)data;

	ASSERT2(v_pipe, " %s :: argument NULL ", __func__);
	v_pipe->pflip_pending = 0;
	/* Count number of VBLANK events */
	levents_capture_event(v_pipe->events[DISPLAY]);
}

static void *fps_count_thread(void *arg)
{
	struct video_pipeline *s = arg;

	struct pollfd fds[] = {
		{.fd = s->drm.fd, .events = POLLIN},
	};

	/* setup drm event context */
	drmEventContext evctx;
	memset(&evctx, 0, sizeof(evctx));
	evctx.version = DRM_EVENT_CONTEXT_VERSION;
	evctx.vblank_handler = drm_event_handler;

	while (poll(fds, ARRAY_SIZE(fds), POLL_TIMEOUT_MSEC) > 0 ) {
		if (fds[0].revents & POLLIN) {
			/* Processes outstanding DRM events on the DRM file-descriptor*/
			int ret = drmHandleEvent(s->drm.fd, &evctx);
			ASSERT2(!ret, "drmHandleEvent failed: %s\n", ERRSTR);
		}
	}

	pthread_exit(NULL);
}

int vlib_change_mode(struct vlib_config *config)
{
	int ret;
	struct filter_s *fs;
	void *(*process_thread_fptr)(void *);

	/* Print requested config */
	vlib_dbg("config: src=%zu, type=%d, mode=%d\n", config->vsrc,
		 config->type, config->mode);

	if (config->vsrc >= vlib_video_src_cnt_get()) {
		vlib_err("invalid video source '%zu'\n",
			 config->vsrc);
		return VLIB_ERROR_INVALID_PARAM;
	}

	/* filter is required when output resolution != input resolution */
	if ((video_setup->w != video_setup->drm.overlay_plane.vlib_plane.width ||
	     video_setup->h != video_setup->drm.overlay_plane.vlib_plane.height) &&
	    config->mode == FILTER_MODE_OFF) {
		vlib_err("invalid filter mode 'OFF' for selected input/output resolutions\n");
		return VLIB_ERROR_INVALID_PARAM;
	}

	/* Stop processing loop */
	if (video_setup->eventloop) {
		/* Set application state */
		video_setup->app_state = MODE_CHANGE;

		/* Stop previous running mode if any */
		ret = vlib_pipeline_term_threads(video_setup);
	}

	const struct vlib_vdev *vdev = vlib_video_src_get(config->vsrc);
	if (!vdev) {
		return VLIB_ERROR_INVALID_PARAM;
	}

	/* Set video source */
	video_setup->vid_src = vdev;

	if (vdev->ops && vdev->ops->change_mode) {
		ret = vdev->ops->change_mode(video_setup, config);
		if (ret) {
			return ret;
		}
	}

	/* Initialize filter mode */
	fs = filter_type_get_obj(video_setup->ft, config->type);
	if (!fs)
		config->mode = FILTER_MODE_OFF;

	const struct stream_handle *sh;
	switch(config->mode) {
	case FILTER_MODE_OFF:
		sh = s2m_pipeline_init(video_setup);
		if (!sh) {
			return VLIB_ERROR_CAPTURE;
		}
		process_thread_fptr = s2m_process_event_loop;
		break;
	case FILTER_MODE_HW:
#if !defined(WITH_SDSOC)
		config->mode = FILTER_MODE_SW;
		vlib_info("No Hardware filter found!\n");
		vlib_info("Continue with previous mode\n");
#endif
		/* fall through */
	case FILTER_MODE_SW:
		filter_type_set_mode(fs, config->mode);
		sh = m2m_sw_pipeline_init(video_setup, fs);
		if (!sh) {
			return VLIB_ERROR_CAPTURE;
		}
		process_thread_fptr = m2m_sw_process_event_loop;
		break;
	default:
		ASSERT2(0, "Invalid application mode!\n");
	}

	/* start fps counter thread */
	ret = pthread_create(&video_setup->fps_thread, NULL, fps_count_thread,
			     video_setup);
	if (ret) {
		vlib_warn("failed to create FPS count thread\n");
	}

	/* Start the processing loop */
	ret = pthread_create(&video_setup->eventloop, NULL, process_thread_fptr,
			     (void *)sh);
	ASSERT2(ret >= 0, "thread creation failed \n");

	return VLIB_SUCCESS;
}

int vlib_drm_set_layer0_state(int enable_state)
{
	/* Map primary-plane cordinates into CRTC using drmModeSetPlane */
	drm_set_plane_state(&video_setup->drm, video_setup->drm.prim_plane.drm_plane.plane_id, enable_state);
	return VLIB_SUCCESS;
}

int vlib_drm_set_layer0_transparency(int transparency)
{
	/* Set Layer Alpha for graphics layer */
	drm_set_plane_prop(&video_setup->drm, video_setup->drm.prim_plane.drm_plane.plane_id,
			DRM_ALPHA_PROP, (DRM_MAX_ALPHA-transparency));

	return VLIB_SUCCESS;
}

int vlib_drm_set_layer0_position(int x, int y)
{
	drm_set_prim_plane_pos(&video_setup->drm, x, y);
	return VLIB_SUCCESS;
}

/* Set event-log state */
int vlib_set_event_log(int state)
{
	video_setup->enable_log_event = state;
	return VLIB_SUCCESS;
}

/**
 * vlib_get_event_cnt - Retrieve normalized event counter
 * @event: Event counter to return
 *
 * Return: Normalized event count for @event.
 */
float vlib_get_event_cnt(pipeline_event event)
{
	if (video_setup->enable_log_event && event < NUM_EVENTS) {
		return levents_counter_get_value(video_setup->events[event]);
	}

	return VLIB_ERROR_OTHER;
}

/** This function returns a constant NULL-terminated string with the ASCII name of a vlib
 *  error. The caller must not free() the returned string.
 *
 *  \param error_code The \ref vlib_error to return the name of.
 *  \returns The error name, or the string **UNKNOWN** if the value of
 *  error_code is not a known error.
 */
const char *vlib_error_name(vlib_error error_code)
{
	switch (error_code) {
	case VLIB_ERROR_INTERNAL:
		return "VLIB Internal Error";
	case VLIB_ERROR_CAPTURE:
		return "VLIB Capture Error";
	case VLIB_ERROR_INVALID_PARAM:
		return "VLIB Invalid Parameter Error";
	case VLIB_ERROR_FILE_IO:
		return "VLIB File I/O Error";
	case VLIB_ERROR_NOT_SUPPORTED:
		return "VLIB Not Supported Error";
	case VLIB_ERROR_OTHER:
		return "VLIB Other Error";
	case VLIB_SUCCESS:
		return "VLIB Success";
	default:
		return "VLIB Unknown Error";
	}
}

/** This function returns a string with a short description of the given error code.
 *  This description is intended for displaying to the end user.
 *
 *  The messages always start with a capital letter and end without any dot.
 *  The caller must not free() the returned string.
 *
 *  \returns a short description of the error code in UTF-8 encoding
 */
char *vlib_strerror(void)
{
	return vlib_errstr;
}

/* This function returns a string with a log-information w.r.t to the input log-level */
static void vlib_log_str(vlib_log_level level, const char *str)
{
	fputs(str, stderr);
	UNUSED(level);
}

void vlib_log_v(vlib_log_level level, const char *format, va_list args)
{
	const char *prefix = "";
	char buf[VLIB_LOG_SIZE];
	int header_len, text_len;

	switch (level) {
	case VLIB_LOG_LEVEL_INFO:
		prefix = "[vlib info] ";
		break;
	case VLIB_LOG_LEVEL_WARNING:
		prefix = "[vlib warning] ";
		break;
	case VLIB_LOG_LEVEL_ERROR:
		prefix = "[vlib error] ";
		break;
	case VLIB_LOG_LEVEL_DEBUG:
		prefix = "[vlib debug] ";
		break;
	case VLIB_LOG_LEVEL_NONE:
	default:
		return;
	}

	header_len = snprintf(buf, sizeof(buf), "%s", prefix);
	if (header_len < 0 || header_len >= (int)sizeof(buf)) {
		/* Somehow snprintf failed to write to the buffer,
		 * remove the header so something useful is output. */
		header_len = 0;
	}
	/* Make sure buffer is NULL terminated */
	buf[header_len] = '\0';

	text_len = vsnprintf(buf + header_len, sizeof(buf) - header_len, format, args);
	if (text_len < 0 || text_len + header_len >= (int)sizeof(buf)) {
		/* Truncated log output. On some platforms a -1 return value means
		 * that the output was truncated. */
		text_len = sizeof(buf) - header_len;
	}

	if (header_len + text_len >= sizeof(buf)) {
		/* Need to truncate the text slightly to fit on the terminator. */
		text_len -= (header_len + text_len) - sizeof(buf);
	}

	vlib_log_str(level, buf);
}

void vlib_log(vlib_log_level level, const char *format, ...)
{
	va_list args;

	va_start (args, format);
	vlib_log_v(level, format, args);
	va_end (args);
}

int vlib_pipeline_v4l2_init(struct stream_handle *sh, struct video_pipeline *s,
			    int mem_type)
{
	/* Initialize v4l2 video input device */
	sh->video_in.fd = vlib_video_src_get_vnode(s->vid_src);
	sh->video_in.format.pixelformat = s->in_fourcc;
	sh->video_in.format.width = s->w;
	sh->video_in.format.height = s->h;
	sh->video_in.format.bytesperline = s->stride;
	sh->video_in.format.colorspace = V4L2_COLORSPACE_SRGB;
	sh->video_in.buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	sh->video_in.mem_type = mem_type;
	sh->video_in.setup_ptr = s;
	return v4l2_init(&sh->video_in, s->buffer_cnt);
}

/**
 * vlib_fourcc2bpp - Get bytes per pixel
 * @fourcc: Fourcc pixel format code
 *
 * Return: Number of bytes per pixel for @fourcc or 0.
 */
size_t vlib_fourcc2bpp(uint32_t fourcc)
{
	size_t bpp;

	/* look up bits per pixel */
	switch (fourcc) {
	case V4L2_PIX_FMT_RGB332:
	case V4L2_PIX_FMT_HI240:
	case V4L2_PIX_FMT_HM12:
	case DRM_FORMAT_RGB332:
	case DRM_FORMAT_BGR233:
		bpp = 8;
		break;
	case V4L2_PIX_FMT_YVU410:
	case V4L2_PIX_FMT_YUV410:
		bpp = 9;
		break;
	case V4L2_PIX_FMT_YVU420:
	case V4L2_PIX_FMT_YUV420:
	case V4L2_PIX_FMT_M420:
	case V4L2_PIX_FMT_Y41P:
		bpp = 12;
		break;
	case V4L2_PIX_FMT_RGB444:
	case V4L2_PIX_FMT_ARGB444:
	case V4L2_PIX_FMT_XRGB444:
	case V4L2_PIX_FMT_RGB555:
	case V4L2_PIX_FMT_ARGB555:
	case V4L2_PIX_FMT_XRGB555:
	case V4L2_PIX_FMT_RGB565:
	case V4L2_PIX_FMT_RGB555X:
	case V4L2_PIX_FMT_ARGB555X:
	case V4L2_PIX_FMT_XRGB555X:
	case V4L2_PIX_FMT_RGB565X:
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_YYUV:
	case V4L2_PIX_FMT_YVYU:
	case V4L2_PIX_FMT_UYVY:
	case V4L2_PIX_FMT_VYUY:
	case V4L2_PIX_FMT_YUV422P:
	case V4L2_PIX_FMT_YUV411P:
	case V4L2_PIX_FMT_YUV444:
	case V4L2_PIX_FMT_YUV555:
	case V4L2_PIX_FMT_YUV565:
	case DRM_FORMAT_XBGR4444:
	case DRM_FORMAT_RGBX4444:
	case DRM_FORMAT_BGRX4444:
	case DRM_FORMAT_ABGR4444:
	case DRM_FORMAT_RGBA4444:
	case DRM_FORMAT_BGRA4444:
	case DRM_FORMAT_XBGR1555:
	case DRM_FORMAT_RGBX5551:
	case DRM_FORMAT_BGRX5551:
	case DRM_FORMAT_ABGR1555:
	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_BGRA5551:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_BGR565:
		bpp = 16;
		break;
	case V4L2_PIX_FMT_BGR666:
		bpp = 18;
		break;
	case V4L2_PIX_FMT_BGR24:
	case V4L2_PIX_FMT_RGB24:
	case DRM_FORMAT_RGB888:
	case DRM_FORMAT_BGR888:
		bpp = 24;
		break;
	case V4L2_PIX_FMT_BGR32:
	case V4L2_PIX_FMT_ABGR32:
	case V4L2_PIX_FMT_XBGR32:
	case V4L2_PIX_FMT_RGB32:
	case V4L2_PIX_FMT_ARGB32:
	case V4L2_PIX_FMT_XRGB32:
	case V4L2_PIX_FMT_YUV32:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_XRGB2101010:
	case DRM_FORMAT_XBGR2101010:
	case DRM_FORMAT_RGBX1010102:
	case DRM_FORMAT_BGRX1010102:
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_BGRA1010102:
		bpp = 32;
		break;
	default:
		return 0;
	}

	/* return bytes required to hold one pixel */
	return (bpp + 7) >> 3;
}
