/******************************************************************************
 * (c) Copyright 2016 Xilinx, Inc. All rights reserved.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <mediactl/mediactl.h>
#include <mediactl/v4l2subdev.h>
#include <unistd.h>

#include <helper.h>
#include <mediactl_helper.h>
#include <v4l2_helper.h>
#include <vcap_hdmi_int.h>

#define MEDIA_G_DVTIMINGS_RETRY_CNT 10
#define MEDIA_G_DVTIMINGS_RETRY_DLY_USEC 100000

#if defined(PLATFORM_ZCU102)
#define MEDIA_ADV7611_ENTITY	"adv7611 25-004c"
#define MEDIA_HDMI_RXSS_ENTITY	"a1000000.hdmi_rxss"
#elif defined (PLATFORM_ZC1751_DC1) || defined(PLATFORM_ZC70X)
#define MEDIA_ADV7611_ENTITY	"adv7611 12-004c"
#define MEDIA_HDMI_RXSS_ENTITY	" "
#endif

#ifdef HDMI_ADV7611
#define MEDIA_HDMI_ENTITY	MEDIA_ADV7611_ENTITY
#define MEDIA_HDMI_FMT_OUT	"UYVY"
#define MEDIA_HDMI_PAD		1
#else /* HDMI_RXSS */
#define MEDIA_HDMI_ENTITY	MEDIA_HDMI_RXSS_ENTITY
#define MEDIA_HDMI_PAD		0
#endif

#if defined(PLATFORM_ZCU102) || defined (PLATFORM_ZC1751_DC1)
#define MEDIA_SCALER_ENTITY	"b0100000.scaler"
#elif defined(PLATFORM_ZC70X)
#define MEDIA_SCALER_ENTITY	" "
#endif
#define MEDIA_SCALER_FMT_OUT	"UYVY"

static int vcap_hdmi_ops_set_media_ctrl(struct video_pipeline *video_setup,
					const struct vlib_vdev *vdev)
{
	int ret = VLIB_SUCCESS;
	struct media_pad *pad;
	struct v4l2_dv_timings timings;
	int retry_cnt = MEDIA_G_DVTIMINGS_RETRY_CNT;
	char fmt_str[100];
	struct media_device *media = vlib_vdev_get_mdev(vdev);
	struct v4l2_mbus_framefmt format;
	const char* fmt_code;

	/* Enumerate entities, pads and links */
	ret = media_device_enumerate(media);
	ASSERT2(ret >= 0, "failed to enumerate %s\n", vdev->display_text);

#ifdef VLIB_LOG_LEVEL_DEBUG
	const struct media_device_info *info = media_get_info(media);
	print_media_info(info);
#endif

	/* Get HDMI Rx pad */
	memset(fmt_str, 0, sizeof(fmt_str));
	media_set_pad_str(fmt_str, MEDIA_HDMI_ENTITY, MEDIA_HDMI_PAD);
	pad = media_parse_pad(media, fmt_str, NULL);
	ASSERT2(pad, "Pad '%s' not found\n", fmt_str);

	/* Repeat query dv_timings as occasionally the reported timings are incorrect */
	do {
		retry_query_timing:
		ret = v4l2_subdev_query_dv_timings(pad->entity, &timings);
		if (ret < 0 && retry_cnt--) {
			/* Delay dv_timings query in-case of failure */
			usleep(MEDIA_G_DVTIMINGS_RETRY_DLY_USEC);
			goto retry_query_timing;
		}
	} while (!video_setup->has_scaler &&
		 (timings.bt.width != video_setup->drm.overlay_plane.vlib_plane.width ||
		 timings.bt.height != video_setup->drm.overlay_plane.vlib_plane.height ||
		 !retry_cnt--));
	ASSERT2(!(ret), "Failed to query DV timings: %s\n", strerror(-ret));
	ASSERT2(!(retry_cnt < 0), "Incorrect HDMI Rx DV timings: %dx%d\n",
		timings.bt.width, timings.bt.height);

	if (retry_cnt < MEDIA_G_DVTIMINGS_RETRY_CNT)
		vlib_dbg("Link to HDMI source recovered (required retries: %d)\n", MEDIA_G_DVTIMINGS_RETRY_CNT-retry_cnt);

#ifdef HDMI_ADV7611
	/* Set HDMI Rx DV timing */
	ret = v4l2_subdev_set_dv_timings(pad->entity, &timings);
	ASSERT2(!(ret < 0), "Failed to set DV timings: %s\n", strerror(-ret));

	/* Set HDMI Rx resolution */
	memset(fmt_str, 0, sizeof(fmt_str));
	media_set_fmt_str(fmt_str, MEDIA_HDMI_ENTITY, 1, MEDIA_HDMI_FMT_OUT,
			  video_setup->hdmi_in_width, video_setup->hdmi_in_height);
	ret = v4l2_subdev_parse_setup_formats(media, fmt_str);
	ASSERT2(!(ret), "Unable to setup formats: %s (%d)\n", strerror(-ret),
		-ret);
#endif

	/* Retrieve HDMI Rx pad format */
	ret = v4l2_subdev_get_format(pad->entity, &format, MEDIA_HDMI_PAD,
				     V4L2_SUBDEV_FORMAT_ACTIVE);
	ASSERT2(!(ret), "Failed to get HDMI Rx pad format: %s\n",
		strerror(-ret));
	fmt_code = v4l2_subdev_pixelcode_to_string(format.code);
	vlib_dbg("HDMI Rx source pad format: %s, %ux%u\n", fmt_code,
		 format.width, format.height);

	/* Set Scaler resolution */
	if (video_setup->has_scaler) {
		memset(fmt_str, 0, sizeof(fmt_str));
		media_set_fmt_str(fmt_str, MEDIA_SCALER_ENTITY, 0, fmt_code,
				  video_setup->hdmi_in_width,
				  video_setup->hdmi_in_height);
		ret = v4l2_subdev_parse_setup_formats(media, fmt_str);
		ASSERT2(!(ret), "Unable to setup formats: %s (%d)\n",
			strerror(-ret), -ret);

		memset(fmt_str, 0, sizeof(fmt_str));
		media_set_fmt_str(fmt_str, MEDIA_SCALER_ENTITY, 1,
				  MEDIA_SCALER_FMT_OUT,
				  video_setup->drm.overlay_plane.vlib_plane.width,
				  video_setup->drm.overlay_plane.vlib_plane.height);
		ret = v4l2_subdev_parse_setup_formats(media, fmt_str);
		ASSERT2(!(ret), "Unable to setup formats: %s (%d)\n",
			strerror(-ret), -ret);
	}

	return ret;
}

static int vcap_hdmi_ops_change_mode(struct video_pipeline *video_setup,
				     struct vlib_config *config)
{
	int ret;
	struct v4l2_dv_timings dv_timings;
	const struct vlib_vdev *vdev = vlib_video_src_get(config->vsrc);

	/* No HDMI Rx detected */
	if (!vdev) {
		sprintf(vlib_errstr, "HDMI Rx not detected");
		vlib_info("Continue with previous mode\n");
		return VLIB_ERROR_CAPTURE;
	}

	/* Query input resolution */
	ret = query_entity_dv_timings(vdev, MEDIA_HDMI_ENTITY, MEDIA_HDMI_PAD,
				      &dv_timings);
	if (ret) {
		sprintf(vlib_errstr, "Query DV timings failed: %s",
			strerror(errno));
		return VLIB_ERROR_CAPTURE;
	}

	video_setup->hdmi_in_width = dv_timings.bt.width;
	video_setup->hdmi_in_height = dv_timings.bt.height;

	if (!video_setup->has_scaler &&
	    (video_setup->hdmi_in_width != video_setup->drm.overlay_plane.vlib_plane.width ||
	    video_setup->hdmi_in_height != video_setup->drm.overlay_plane.vlib_plane.height)) {
		sprintf(vlib_errstr, "HDMI input resolution '%dx%d' does not match plane or display '%lux%lu'",
			video_setup->hdmi_in_width,
			video_setup->hdmi_in_height,
			video_setup->drm.overlay_plane.vlib_plane.width,
			video_setup->drm.overlay_plane.vlib_plane.height);
		vlib_info("Continue with previous mode\n");
		return VLIB_ERROR_CAPTURE;
	}

	return 0;
}

static const struct vsrc_ops vcap_hdmi_ops = {
	.change_mode = vcap_hdmi_ops_change_mode,
	.set_media_ctrl = vcap_hdmi_ops_set_media_ctrl,
};

struct vlib_vdev *vcap_hdmi_init(const struct matchtable *mte, void *media)
{
	struct vlib_vdev *vd = calloc(1, sizeof(*vd));
	if (!vd) {
		return NULL;
	}

	vd->vsrc_type = VSRC_TYPE_MEDIA;
	vd->data.media.mdev = media;
	vd->vsrc_class = VLIB_VCLASS_HDMII;
	vd->display_text = "HDMI Input";
	vd->entity_name = mte->s;
	vd->ops = &vcap_hdmi_ops;

	const char *fn = vlib_video_src_mdev2vdev(vd->data.media.mdev);
	vd->data.media.vnode = open(fn, O_RDWR);
	if (vd->data.media.vnode < 0) {
		free(vd);
		return NULL;
	}

	return vd;
}
