/******************************************************************************
 * (c) Copyright 2017 Xilinx, Inc. All rights reserved.
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
#include <stdlib.h>
#include <sys/ioctl.h>
#include <mediactl/mediactl.h>
#include <mediactl/v4l2subdev.h>
#include <unistd.h>

#include <helper.h>
#include <mediactl_helper.h>
#include <v4l2_helper.h>
#include <vcap_csi_int.h>

#ifdef SENSOR_OV13850
#include "ov13850_cfg.h"
#define MEDIA_SENSOR_ENTITY	MEDIA_OV13850_ENTITY
#define MEDIA_SENSOR_FMT_OUT	MEDIA_OV13850_FMT_OUT
#else /* SENSOR_IMX274 */
#include "imx274_cfg.h"
#define MEDIA_SENSOR_ENTITY	MEDIA_IMX274_ENTITY
#define MEDIA_SENSOR_FMT_OUT	MEDIA_IMX274_FMT_OUT
#endif

#define MEDIA_CSI_ENTITY	"a0060000.csiss"
#define MEDIA_CSI_FMT_IN	MEDIA_SENSOR_FMT_OUT
#define MEDIA_CSI_FMT_OUT	MEDIA_CSI_FMT_IN

#define MEDIA_DMSC_ENTITY	"b0040000.v_demosaic"
#define MEDIA_DMSC_FMT_IN	MEDIA_CSI_FMT_OUT
#define MEDIA_DMSC_FMT_OUT	"RBG24"

#define MEDIA_GAMMA_ENTITY	"b0010000.v_gamma"
#define MEDIA_GAMMA_FMT_IN	MEDIA_DMSC_FMT_OUT
#define MEDIA_GAMMA_FMT_OUT	MEDIA_GAMMA_FMT_IN

#define MEDIA_CSC_ENTITY	"b0060000.csc"
#define MEDIA_CSC_FMT_IN	MEDIA_GAMMA_FMT_OUT
#define MEDIA_CSC_FMT_OUT	MEDIA_CSC_FMT_IN

#define MEDIA_SCALER_ENTITY	"b0080000.scaler"
#define MEDIA_SCALER_FMT_IN	MEDIA_CSC_FMT_OUT
#define MEDIA_SCALER_FMT_OUT	"UYVY"

#define CSI_ACT_LANES	4

static unsigned int act_lanes = CSI_ACT_LANES;

static int v4l2_csi_set_ctrl(const struct vlib_vdev *vd, int id, int value)
{
	return v4l2_set_ctrl(vd, MEDIA_CSI_ENTITY, id, value);
}

static void csi_set_act_lanes(const struct vlib_vdev *vd, unsigned int lanes)
{
	v4l2_csi_set_ctrl(vd, V4L2_CID_XILINX_MIPICSISS_ACT_LANES, lanes);
	act_lanes = lanes;
}

#define GAMMA_BLUE_COR	10 /* 10 equals passthrough */
#define GAMMA_GREEN_COR	10 /* 10 equals passthrough */
#define GAMMA_RED_COR	10 /* 10 equals passthrough */

static unsigned int blue_cor = GAMMA_BLUE_COR;
static unsigned int green_cor = GAMMA_GREEN_COR;
static unsigned int red_cor = GAMMA_RED_COR;

static int v4l2_gamma_set_ctrl(const struct vlib_vdev *vd, int id, int value)
{
	return v4l2_set_ctrl(vd, MEDIA_GAMMA_ENTITY, id, value);
}

void gamma_set_blue_correction(const struct vlib_vdev *vd, unsigned int blue)
{
	v4l2_gamma_set_ctrl(vd, V4L2_CID_XILINX_GAMMA_CORR_BLUE_GAMMA, blue);
	blue_cor = blue;
}

void gamma_set_green_correction(const struct vlib_vdev *vd, unsigned int green)
{
	v4l2_gamma_set_ctrl(vd, V4L2_CID_XILINX_GAMMA_CORR_GREEN_GAMMA, green);
	green_cor = green;
}

void gamma_set_red_correction(const struct vlib_vdev *vd, unsigned int red)
{
	v4l2_gamma_set_ctrl(vd, V4L2_CID_XILINX_GAMMA_CORR_RED_GAMMA, red);
	red_cor = red;
}

#define CSC_BRIGHTNESS	50
#define CSC_CONTRAST	50
#define CSC_BLUE_GAIN	50
#define CSC_GREEN_GAIN	50
#define CSC_RED_GAIN	50

static unsigned int brightness = CSC_BRIGHTNESS;
static unsigned int contrast = CSC_CONTRAST;
static unsigned int blue_gain = CSC_BLUE_GAIN;
static unsigned int green_gain = CSC_GREEN_GAIN;
static unsigned int red_gain = CSC_RED_GAIN;

static int v4l2_csc_set_ctrl(const struct vlib_vdev *vd, int id, int value)
{
	return v4l2_set_ctrl(vd, MEDIA_CSC_ENTITY, id, value);
}

void csc_set_brightness(const struct vlib_vdev *vd, unsigned int bright)
{
	v4l2_csc_set_ctrl(vd, V4L2_CID_XILINX_CSC_BRIGHTNESS, bright);
	brightness = bright;
}

void csc_set_contrast(const struct vlib_vdev *vd, unsigned int cont)
{
	v4l2_csc_set_ctrl(vd, V4L2_CID_XILINX_CSC_CONTRAST, cont);
	contrast = cont;
}

void csc_set_blue_gain(const struct vlib_vdev *vd, unsigned int blue)
{
	v4l2_csc_set_ctrl(vd, V4L2_CID_XILINX_CSC_BLUE_GAIN, blue);
	blue_gain = blue;
}

void csc_set_green_gain(const struct vlib_vdev *vd, unsigned int green)
{
	v4l2_csc_set_ctrl(vd, V4L2_CID_XILINX_CSC_GREEN_GAIN, green);
	green_gain = green;
}

void csc_set_red_gain(const struct vlib_vdev *vd, unsigned int red)
{
	v4l2_csc_set_ctrl(vd, V4L2_CID_XILINX_CSC_RED_GAIN, red);
	red_gain = red;
}

static void __attribute__((__unused__)) csi_log_status(const struct vlib_vdev *vdev)
{
        int fd, ret;
        char subdev_name[DEV_NAME_LEN];

        get_entity_devname(vlib_vdev_get_mdev(vdev), MEDIA_CSI_ENTITY,
			   subdev_name);

        fd = open(subdev_name, O_RDWR);
        ASSERT2(fd >= 0, "failed to open %s: %s\n", subdev_name, ERRSTR);

        ret = ioctl(fd, VIDIOC_LOG_STATUS);
        ASSERT2(ret >= 0, "VIDIOC_LOG_STATUS failed: %s\n", ERRSTR);

        close(fd);
}

static int vcap_csi_ops_set_media_ctrl(struct video_pipeline *video_setup,
				       const struct vlib_vdev *vdev)
{
	int ret;
	char media_formats[100];
	struct media_device *media = vlib_vdev_get_mdev(vdev);

	/* Enumerate entities, pads and links */
	ret = media_device_enumerate(media);
	ASSERT2(ret >= 0, "failed to enumerate %s\n", vdev->display_text);

#ifdef VLIB_LOG_LEVEL_DEBUG
	const struct media_device_info *info = media_get_info(media);
	print_media_info(info);
#endif

	/* Set image sensor format */
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_SENSOR_ENTITY, 0,
			  MEDIA_SENSOR_FMT_OUT,
			  video_setup->drm.overlay_plane.vlib_plane.width,
			  video_setup->drm.overlay_plane.vlib_plane.height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats: %s (%d)\n", strerror(-ret),
		-ret);

	/* Set MIPI CSI2 Rx format */
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_CSI_ENTITY, 0, MEDIA_CSI_FMT_IN,
			  video_setup->drm.overlay_plane.vlib_plane.width,
			  video_setup->drm.overlay_plane.vlib_plane.height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats: %s (%d)\n", strerror(-ret),
		-ret);

	/* Set Demosaic format */
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_DMSC_ENTITY, 0,
			  MEDIA_DMSC_FMT_IN,
			  video_setup->drm.overlay_plane.vlib_plane.width,
			  video_setup->drm.overlay_plane.vlib_plane.height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats: %s (%d)\n", strerror(-ret),
		-ret);
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_DMSC_ENTITY, 1,
			  MEDIA_DMSC_FMT_OUT,
			  video_setup->drm.overlay_plane.vlib_plane.width,
			  video_setup->drm.overlay_plane.vlib_plane.height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats: %s (%d)\n", strerror(-ret),
		-ret);

	/* Set Gamma format */
	memset (media_formats, 0, sizeof (media_formats));
	media_set_fmt_str(media_formats, MEDIA_GAMMA_ENTITY, 0,
			  MEDIA_GAMMA_FMT_IN,
			  video_setup->drm.overlay_plane.vlib_plane.width,
			  video_setup->drm.overlay_plane.vlib_plane.height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats: %s (%d)\n", strerror(-ret),
		-ret);
	memset (media_formats, 0, sizeof (media_formats));
	media_set_fmt_str(media_formats, MEDIA_GAMMA_ENTITY, 1,
			  MEDIA_GAMMA_FMT_OUT,
			  video_setup->drm.overlay_plane.vlib_plane.width,
			  video_setup->drm.overlay_plane.vlib_plane.height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats: %s (%d)\n", strerror(-ret),
		-ret);

	/* Set CSC format */
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_CSC_ENTITY, 0,
			  MEDIA_CSC_FMT_IN,
			  video_setup->drm.overlay_plane.vlib_plane.width,
			  video_setup->drm.overlay_plane.vlib_plane.height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats: %s (%d)\n", strerror(-ret),
		-ret);
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_CSC_ENTITY, 1,
			  MEDIA_CSC_FMT_OUT,
			  video_setup->drm.overlay_plane.vlib_plane.width,
			  video_setup->drm.overlay_plane.vlib_plane.height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats: %s (%d)\n", strerror(-ret),
		-ret);

	/* Set Scaler format */
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_SCALER_ENTITY, 0,
			  MEDIA_SCALER_FMT_IN,
			  video_setup->drm.overlay_plane.vlib_plane.width,
			  video_setup->drm.overlay_plane.vlib_plane.height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats: %s (%d)\n", strerror(-ret),
		-ret);
	memset(media_formats, 0, sizeof(media_formats));
	media_set_fmt_str(media_formats, MEDIA_SCALER_ENTITY, 1,
			  MEDIA_SCALER_FMT_OUT,
			  video_setup->drm.overlay_plane.vlib_plane.width,
			  video_setup->drm.overlay_plane.vlib_plane.height);
	ret = v4l2_subdev_parse_setup_formats(media, media_formats);
	ASSERT2(!ret, "Unable to setup formats: %s (%d)\n", strerror(-ret),
		-ret);

	return ret;
}

static int vcap_csi_ops_change_mode(struct video_pipeline *video_setup,
				    struct vlib_config *config)
{
#ifdef SENSOR_OV13850
	return ov13850_setup(video_setup->drm.overlay_plane.vlib_plane.width,
			     video_setup->drm.overlay_plane.vlib_plane.height,
			     VLIB_PPC_SINGLE);
#else /* SENSOR_IMX274 */
	return imx274_setup(video_setup->drm.overlay_plane.vlib_plane.width,
			    video_setup->drm.overlay_plane.vlib_plane.height,
			    video_setup->fps.numerator);
#endif
}

/* TODO: Remove once subdevice driver for Leopard sensor becomes available */
static int vcap_csi_ops_streamon(void)
{
	/* Start image sensor */
#ifdef SENSOR_OV13850
	int ret = ov13850_start();
#else /* SENSOR_IMX274 */
	int ret = imx274_start();
#endif
	ASSERT2(!ret, "Unable to start sensor\n");

	return ret;
}

/* TODO: Remove once subdevice driver for Leopard sensor becomes available */
static int vcap_csi_ops_streamoff(void)
{
	/* Stop image sensor */
#ifdef SENSOR_OV13850
	int ret = ov13850_stop();
#else
	int ret = imx274_stop();
#endif
	ASSERT2(!ret, "Unable to stop sensor\n");

	return ret;
}

static const struct vsrc_ops vcap_csi_ops = {
	.change_mode = vcap_csi_ops_change_mode,
	.set_media_ctrl = vcap_csi_ops_set_media_ctrl,
	.streamon = vcap_csi_ops_streamon,
	.streamoff = vcap_csi_ops_streamoff,
};

struct vlib_vdev *vcap_csi_init(const struct matchtable *mte, void *media)
{
	struct vlib_vdev *vd = calloc(1, sizeof(*vd));
	if (!vd) {
		return NULL;
	}

	vd->vsrc_type = VSRC_TYPE_MEDIA;
	vd->data.media.mdev = media;
	vd->vsrc_class = VLIB_VCLASS_CSI;
	vd->display_text = "MIPI CSI2 Rx";
	vd->entity_name = mte->s;
	vd->ops = &vcap_csi_ops;

	vd->data.media.vnode = open(vlib_video_src_mdev2vdev(vd->data.media.mdev), O_RDWR);
	if (vd->data.media.vnode < 0) {
		free(vd);
		return NULL;
	}

	/* Set active number of lanes */
	csi_set_act_lanes(vd, act_lanes);

	/* Set gamma correction */
	gamma_set_blue_correction(vd, blue_cor);
	gamma_set_green_correction(vd, green_cor);
	gamma_set_red_correction(vd, red_cor);

	/* Set CSC defaults */
	csc_set_brightness(vd, brightness);
	csc_set_contrast(vd, contrast);
	csc_set_blue_gain(vd, blue_gain);
	csc_set_green_gain(vd, green_gain);
	csc_set_red_gain(vd, red_gain);

	return vd;
}
