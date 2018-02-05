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
#include <stdio.h>

#include <platform.h>
#include <video_int.h>

static uint8_t *vcap_file_get_frame(const struct vlib_vdev *vdev,
				    const struct video_pipeline *vp)
{
	uint8_t *buf;
	size_t bpp = vlib_fourcc2bpp(vp->in_fourcc);
	struct vlib_vdev *vd = (struct vlib_vdev *)vdev;

	ASSERT2(bpp, "invalid pixel format '.4%s'\n", (const char *)&vp->in_fourcc);

	buf = vd->data.file.buf + vp->w * vp->h * bpp * vdev->data.file.buf_cur;

	vd->data.file.buf_cur++;
	vd->data.file.buf_cur %= vdev->data.file.buf_cnt;

	return buf;
}

static size_t get_file_size(FILE *fd)
{
	int ret = fseek(fd, 0, SEEK_END);
	if (ret) {
		vlib_err("unable to get file size\n");
		return 0;
	}

	size_t sz = ftell(fd);

	ret = fseek(fd, 0, SEEK_SET);
	if (ret) {
		vlib_err("unable to set position indicator\n");
	}

	return sz;
}

static int vcap_file_prepare(struct video_pipeline *vp, const struct vlib_vdev *vdev)
{
	int ret;
	struct vlib_vdev *vd = (struct vlib_vdev *)vdev;
	size_t frame_sz, frame_cnt, bpp = vlib_fourcc2bpp(vp->in_fourcc);
	ASSERT2(bpp, "invalid pixel format '%.4s'\n", (const char *)&vp->in_fourcc);

	frame_sz = vp->w * vp->h * bpp;
	frame_cnt = get_file_size(vdev->data.file.fd) / frame_sz;

	if (!frame_cnt || !frame_sz) {
		vlib_err("no input data\n");
		return VLIB_ERROR_FILE_IO;
	}

	vd->data.file.buf = calloc(frame_cnt, frame_sz);
	if (!vd->data.file.buf) {
		vlib_err("unable to allocate memory\n");
		return VLIB_ERROR_NO_MEM;
	}

	ret = fread(vd->data.file.buf, frame_sz, frame_cnt, vd->data.file.fd);
	if (ret != frame_cnt) {
		vlib_warn("not enough input data\n");
	}

	vd->data.file.buf_cnt = ret;
	vd->data.file.buf_cur = 0;

	return 0;
}

static int vcap_file_unprepare(struct video_pipeline *vp, const struct vlib_vdev *vdev)
{
	struct vlib_vdev *vd = (struct vlib_vdev *)vdev;

	vd->data.file.buf_cnt = 0;
	free(vdev->data.file.buf);

	return 0;
}

static const struct vsrc_ops vcap_file_ops = {
	.prepare = vcap_file_prepare,
	.unprepare = vcap_file_unprepare,
};

struct vlib_vdev *vcap_file_init(const struct matchtable *mte, const void *filename)
{
	const char *fn = filename;
	struct vlib_vdev *vd = calloc(1, sizeof(*vd));
	if (!vd) {
		return NULL;
	}

	vd->vsrc_type = VSRC_TYPE_FILE;
	vd->data.file.filename = fn;
	vd->vsrc_class = VLIB_VCLASS_FILE;
	vd->display_text = "File reader";
	vd->ops = &vcap_file_ops;
	vd->data.file.buf_cnt = 0;
	vd->data.file.get_frame = vcap_file_get_frame;

	vd->data.file.fd = fopen(fn, "r");
	if (!vd->data.file.fd) {
		vlib_err("unable to open file '%s'\n", fn);
		return NULL;
	}

	return vd;
}
