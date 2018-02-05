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
#include <glib.h>
#include <glob.h>
#include <mediactl/mediactl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <common.h>
#include <helper.h>
#include <mediactl_helper.h>
#include <vcap_hdmi_int.h>
#include <vcap_file_int.h>
#include <vcap_tpg_int.h>
#include <vcap_csi_int.h>
#include <vcap_uvc_int.h>
#include <vcap_vivid_int.h>
#include <video_int.h>

static GPtrArray *video_srcs;

const char *vlib_video_src_get_display_text(const struct vlib_vdev *vsrc)
{
	if (!vsrc) {
		return NULL;
	}

	return vsrc->display_text;
}

const char *vlib_video_src_get_entity_name(const struct vlib_vdev *vsrc)
{
	if (!vsrc) {
		return NULL;
	}

	return vsrc->entity_name;
}

int vlib_video_src_get_vnode(const struct vlib_vdev *vsrc)
{
	if (!vsrc) {
		return -VLIB_ERROR_INVALID_PARAM;
	}

	switch (vsrc->vsrc_type) {
	case VSRC_TYPE_MEDIA:
		return vsrc->data.media.vnode;
	case VSRC_TYPE_V4L2:
		return vsrc->data.v4l2.vnode;
	default:
		break;
	}

	return -VLIB_ERROR_INVALID_PARAM;
}

size_t vlib_video_src_get_index(const struct vlib_vdev *vsrc)
{
	if (!vsrc) {
		goto invalid_param;
	}

	for (size_t i = 0; i < video_srcs->len; ++i) {
		const struct vlib_vdev *tmp = g_ptr_array_index(video_srcs, i);

		if (tmp == vsrc)
			return i;
	}

invalid_param:
	sprintf(vlib_errstr, "Invalid parameter");
	return VLIB_ERROR_INVALID_PARAM;
}

enum vlib_vsrc_class vlib_video_src_get_class(const struct vlib_vdev *vsrc)
{
	if (!vsrc) {
		return -VLIB_ERROR_INVALID_PARAM;
	}

	return vsrc->vsrc_class;
}

const struct vlib_vdev *vlib_video_src_get(size_t id)
{
	if (id >= video_srcs->len) {
		return NULL;
	}

	return g_ptr_array_index(video_srcs, id);
}

struct media_device *vlib_vdev_get_mdev(const struct vlib_vdev *vdev)
{
	if (vdev->vsrc_type != VSRC_TYPE_MEDIA) {
		return NULL;
	}

	return vdev->data.media.mdev;
}

static void vlib_vsrc_vdev_free(struct vlib_vdev *vd)
{
	switch (vd->vsrc_type) {
	case VSRC_TYPE_MEDIA:
		media_device_unref(vd->data.media.mdev);
		close(vd->data.media.vnode);
		break;
	case VSRC_TYPE_V4L2:
		close(vd->data.v4l2.vnode);
		break;
	default:
		break;
	}

	free(vd);
}

static void vlib_vsrc_table_free_func(void *e)
{
	struct vlib_vdev *vd = e;

	vlib_vsrc_vdev_free(vd);
}

static void vlib_video_src_disable(struct vlib_vdev *vsrc)
{
	g_ptr_array_remove_fast(video_srcs, vsrc);
}

void vlib_video_src_class_disable(enum vlib_vsrc_class class)
{
	for (size_t i = video_srcs->len; i > 0; i--) {
		struct vlib_vdev *vd = g_ptr_array_index(video_srcs, i - 1);

		if (vd->vsrc_class == class) {
			vlib_video_src_disable(vd);
		}
	}
}

static const struct matchtable mt_entities[] = {
	{
		.s = "vcap_tpg output 0", .init = vcap_tpg_init,
	},
	{
		.s = "vcap_hdmi output 0", .init = vcap_hdmi_init,
	},
	{
		.s = "vcap_csi output 0", .init = vcap_csi_init,
	},
};

static struct vlib_vdev *init_xvideo(const struct matchtable *mte, void *media)
{
	struct vlib_vdev *vd = NULL;
	size_t nents = media_get_entities_count(media);

	for (size_t i = 0; i < nents; i++) {
		const struct media_entity_desc *info;
		struct media_entity *entity = media_get_entity(media, i);

		if (!entity) {
			vlib_warn("failed to get entity %zu\n", i);
			continue;
		}

		info = media_entity_get_info(entity);
		for (size_t j = 0; j < ARRAY_SIZE(mt_entities); j++) {
			if (!strcmp(mt_entities[j].s, info->name)) {
				vd = mt_entities[j].init(&mt_entities[j], media);
				break;
			}
		}
	}

	return vd;
}

static const struct matchtable mt_drivers_media[] = {
	{
		.s = "xilinx-video", .init = init_xvideo,
	},
#ifdef ENABLE_VCAP_UVC
	{
		.s = "uvcvideo", .init = vcap_uvc_init,
	},
#endif
};

static const struct matchtable mt_drivers_v4l2[] = {
#ifdef ENABLE_VCAP_VIVID
	{
		.s = "vivid", .init = vcap_vivid_init,
	},
#endif
};

int vlib_video_src_init(struct vlib_config_data *cfg)
{
	glob_t pglob;
	int ret = glob("/dev/media*", 0, NULL, &pglob);

	if (ret && ret != GLOB_NOMATCH) {
		ret = VLIB_ERROR_OTHER;
		goto error;
	}

	video_srcs = g_ptr_array_new_with_free_func(vlib_vsrc_table_free_func);
	if (!video_srcs) {
		return VLIB_ERROR_OTHER;
	}

	for (size_t i = 0; i < pglob.gl_pathc; i++) {
		struct media_device *media = media_device_new(pglob.gl_pathv[i]);

		if (!media) {
			vlib_warn("failed to create media device from '%s'\n",
				  pglob.gl_pathv[i]);
			continue;
		}

		ret = media_device_enumerate(media);
		if (ret < 0) {
			vlib_warn("failed to enumerate '%s'\n",
				  pglob.gl_pathv[i]);
			media_device_unref(media);
			continue;
		}

		const struct media_device_info *info = media_get_info(media);

		size_t j;
		for (j = 0; j < ARRAY_SIZE(mt_drivers_media); j++) {
			if (strcmp(mt_drivers_media[j].s, info->driver)) {
				continue;
			}

			struct vlib_vdev *vd =
				  mt_drivers_media[j].init(&mt_drivers_media[j],
							   media);
			if (vd) {
				vlib_dbg("found video source '%s (%s)'\n",
					 vd->display_text, pglob.gl_pathv[i]);
				g_ptr_array_add(video_srcs, vd);
				break;
			}
		}

		if (j == ARRAY_SIZE(mt_drivers_media)) {
			media_device_unref(media);
		}
	}

	globfree(&pglob);

	ret = glob("/dev/video*", 0, NULL, &pglob);
	if (ret && ret != GLOB_NOMATCH) {
		ret = VLIB_ERROR_OTHER;
		goto error;
	}

	ret = VLIB_SUCCESS;

	for (size_t i = 0; i < pglob.gl_pathc; i++) {
		int fd = open(pglob.gl_pathv[i], O_RDWR);
		if (fd < 0) {
			ret = VLIB_ERROR_OTHER;
			goto error;
		}

		struct v4l2_capability vcap;
		ret = ioctl(fd, VIDIOC_QUERYCAP, &vcap);
		if (ret) {
			close(fd);
			continue;
		}

		size_t j;
		for (j = 0; j < ARRAY_SIZE(mt_drivers_v4l2); j++) {
			if (strcmp(mt_drivers_v4l2[j].s, (char *)vcap.driver)) {
				continue;
			}

			struct vlib_vdev *vd =
				    mt_drivers_v4l2[j].init(&mt_drivers_v4l2[j],
							    (void *)(uintptr_t)fd);
			if (vd) {
				vlib_dbg("found video source '%s (%s)'\n",
					 vd->display_text, pglob.gl_pathv[i]);
				g_ptr_array_add(video_srcs, vd);
				break;
			}
		}

		if (j == ARRAY_SIZE(mt_drivers_v4l2)) {
			close(fd);
		}
	}

	if (cfg->vcap_file_fn) {
		struct vlib_vdev *vd = vcap_file_init(NULL, (void *)cfg->vcap_file_fn);
		if (vd) {
			vlib_dbg("found video source '%s (%s)'\n",
				 vd->display_text, cfg->vcap_file_fn);
			g_ptr_array_add(video_srcs, vd);
		}
	}

error:
	globfree(&pglob);
	return ret;
}

void vlib_video_src_uninit(void)
{
	g_ptr_array_free(video_srcs, TRUE);
}

size_t vlib_video_src_cnt_get(void)
{
	return video_srcs->len;
}

const char *vlib_video_src_mdev2vdev(struct media_device *media)
{
	struct media_entity *ent = media_get_entity(media, 0);
	if (!ent) {
		return NULL;
	}

	return media_entity_get_devname(ent);
}
