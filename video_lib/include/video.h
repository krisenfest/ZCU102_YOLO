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

#ifndef VDF_LIB_H
#define VDF_LIB_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

#include <helper.h>

struct filter_tbl;

/* Common interface for video library*/
typedef enum {
	DRM_MODULE_XILINX,
	DRM_MODULE_XYLON,
	DRM_MODULE_NONE = -1,
} drm_module;

/* FIXME: remove global variable */
extern drm_module module_g;

typedef enum {
	VIDEO_CTRL_OFF,
	VIDEO_CTRL_ON,
	VIDEO_CTRL_AUTO
} video_ctrl;

typedef enum {
	CAPTURE,
	DISPLAY,
	PROCESS_IN,
	PROCESS_OUT,
	NUM_EVENTS
} pipeline_event;

enum vlib_vsrc_class {
	VLIB_VCLASS_VIVID,
	VLIB_VCLASS_UVC,
	VLIB_VCLASS_TPG,
	VLIB_VCLASS_HDMII,
	VLIB_VCLASS_CSI,
	VLIB_VCLASS_FILE,
};

#include <filter.h>

struct vlib_config {
	size_t vsrc;
	unsigned int type;
	filter_mode mode;
};

#include <linux/videodev2.h>
#include <common.h>

struct vlib_config_data {
	struct filter_tbl *ft;	/* filter table */
	int width_in;		/* input width */
	int height_in;		/* input height */
	uint32_t fmt_in;	/* input pixel format */
	unsigned int flags;	/* flags */
	unsigned int dri_card_id;	/* dri card number */
	int width_out;		/* output width */
	int height_out;		/* output height */
	uint32_t fmt_out;	/* output pixel format */
	struct v4l2_fract fps;	/* frames per second */
	const char *vcap_file_fn;	/* filename for file source */
	struct vlib_plane plane;
	size_t vrefresh;	/* vertical refresh rate */
	const char *drm_background;	/* path to background image */
	size_t buffer_cnt;	/* number of frame buffers */
};

#define VLIB_CFG_FLAG_PR_ENABLE		BIT(0) /* enable partial reconfiguration */
#define VLIB_CFG_FLAG_MULTI_INSTANCE	BIT(1) /* enable multi-instance mode */

/**
 * Error codes. Most vlib functions return 0 on success or one of these
 * codes on failure.
 * User can call vlib_error_name() to retrieve a string representation of an
 * error code or vlib_strerror() to get an end-user suitable description of
 * an error code.
*/

/* Total number of error codes in enum vlib_error */
#define VLIB_ERROR_COUNT 6

typedef enum {
	VLIB_SUCCESS = 0,
	VLIB_ERROR_INTERNAL = -1,
	VLIB_ERROR_CAPTURE = -2,
	VLIB_ERROR_INVALID_PARAM = -3,
	VLIB_ERROR_FILE_IO = -4,
	VLIB_ERROR_NOT_SUPPORTED = -5,
	VLIB_ERROR_NO_MEM = -6,
	VLIB_ERROR_OTHER = -99
} vlib_error;

/* Character-array to store string-representation of the error-codes */
#define VLIB_ERRSTR_SIZE 256
extern char vlib_errstr[VLIB_ERRSTR_SIZE];

/**
 *  Log message levels.
 *  - VLIB_LOG_LEVEL_NONE (0)
 *  - VLIB_LOG_LEVEL_ERROR (1)
 *  - VLIB_LOG_LEVEL_WARNING (2)
 *  - VLIB_LOG_LEVEL_INFO (3)
 *  - VLIB_LOG_LEVEL_DEBUG (4)
 *  All the messages are printed on stderr.
 */
typedef enum {
	VLIB_LOG_LEVEL_NONE = 0,
	VLIB_LOG_LEVEL_ERROR,
	VLIB_LOG_LEVEL_WARNING,
	VLIB_LOG_LEVEL_INFO,
	VLIB_LOG_LEVEL_DEBUG,
} vlib_log_level;

struct video_resolution {
	unsigned int height;
	unsigned int width;
	unsigned int stride;
};

/* The following is used to silence warnings for unused variables */
#define UNUSED(var)		do { (void)(var); } while(0)

/* video source helper functions */
struct vlib_vdev;

const char *vlib_video_src_get_display_text(const struct vlib_vdev *vsrc);
const char *vlib_video_src_get_entity_name(const struct vlib_vdev *vsrc);
enum vlib_vsrc_class vlib_video_src_get_class(const struct vlib_vdev *vsrc);
size_t vlib_video_src_get_index(const struct vlib_vdev *vsrc);
const struct vlib_vdev *vlib_video_src_get(size_t id);
size_t vlib_video_src_cnt_get(void);

static inline const char *vlib_video_src_get_display_text_from_id(size_t id)
{
	const struct vlib_vdev *v = vlib_video_src_get(id);
	return vlib_video_src_get_display_text(v);
}

static inline const char *vlib_video_src_get_entity_name_from_id(size_t id)
{
	const struct vlib_vdev *v = vlib_video_src_get(id);
	return vlib_video_src_get_entity_name(v);
}

/* drm helper functions */
int vlib_drm_set_layer0_state(int);
int vlib_drm_set_layer0_transparency(int);
int vlib_drm_set_layer0_position(int, int);
int vlib_drm_try_mode(unsigned int dri_card_id, int width, int height);

/* video resolution functions */
int vlib_get_active_height(void);
int vlib_get_active_width(void);

/* init/uninit functions */
int vlib_init(struct vlib_config_data *cfg);
int vlib_uninit(void);

/* video pipeline control functions */
int vlib_pipeline_stop(void);
int vlib_change_mode(struct vlib_config *config);

/* set event-log function */
int vlib_set_event_log(int state);
/* Query pipeline events*/
float vlib_get_event_cnt(pipeline_event event);

/* return the string representation of the error code */
const char *vlib_error_name(vlib_error error_code);
/* return user-readable description of the error-code */
char *vlib_strerror(void);

#ifdef __cplusplus
}
#endif

#endif /* VDF_LIB_H */
