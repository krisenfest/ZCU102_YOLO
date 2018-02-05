/******************************************************************************
 * (c) Copyright 2012-2017 Xilinx, Inc. All rights reserved.
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
#include <glib.h>
#include <libdrm/drm.h>
#include <libdrm/drm_mode.h>
#include <poll.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "drm_helper.h"
#include "filter.h"
#include "helper.h"
#include "log_events.h"
#include "platform.h"
#include "m2m_sw_pipeline.h"
#include "video.h"

#define M2M_SW_PIPELINE_DELAY_SRC2FILTER	1
#define M2M_SW_PIPELINE_DELAY_SINK2SRC		1

const struct stream_handle *m2m_sw_pipeline_init(struct video_pipeline *s,
						 struct filter_s *fs)
{
	int ret = 0;
	const struct vlib_vdev *vdev = s->vid_src;
	ASSERT2(vdev, "invalid video source\n");

	/* Configure media pipelines */
	if (vdev->ops && vdev->ops->set_media_ctrl) {
		ret = vdev->ops->set_media_ctrl(s, vdev);
		ASSERT2(!ret, "failed to configure media pipeline\n");
	}

	if (vdev->ops && vdev->ops->prepare) {
		ret = vdev->ops->prepare(s, vdev);
		vlib_err("preparing video source failed\n");
		return NULL;
	}

	struct stream_handle *sh = calloc(1, sizeof(*sh));
	if (!sh) {
		return NULL;
	}

	sh->vp = s;

	if (video_src_is_v4l2(vdev)) {
		ret = vlib_pipeline_v4l2_init(sh, s, V4L2_MEMORY_DMABUF);
		if (ret) {
			vlib_err("init v4l2 pipeline failed\n");
			free(sh);
			return NULL;
		}
	}

	/* Initialize video output format */
	sh->video_out.width = s->drm.overlay_plane.vlib_plane.width;
	sh->video_out.height = s->drm.overlay_plane.vlib_plane.height;
	sh->video_out.stride = s->stride_out;

	/* Initialize filter */
	sh->fs = fs;

	return sh;
}

static void m2m_sw_v4l2_cleanup(void *arg)
{
	struct stream_handle *sh = arg;

	g_queue_free(sh->buffer_q_src2filter);
	g_queue_free(sh->buffer_q_filter2sink);
	g_queue_free(sh->buffer_q_sink2src);
}

static void m2m_sw_v4l2_process_loop(struct stream_handle *sh)
{
	int ret;
	struct video_pipeline *v_pipe = sh->vp;
	/* TODO: Remove once image sensor subdevice driver becomes available */
	const struct vlib_vdev *vdev = v_pipe->vid_src;
	ASSERT2(vdev, "invalid video source\n");

	for (size_t i = 0; i < v_pipe->buffer_cnt; ++i) {
		struct v4l2_buffer buffer;
		memset(&buffer, 0, sizeof(buffer));
		buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buffer.memory = V4L2_MEMORY_DMABUF;
		buffer.index = i;
		ret = ioctl(sh->video_in.fd, VIDIOC_QUERYBUF, &buffer);
		if (ret) {
			perror("VIDIOC_QUERYBUF");
			exit(EXIT_FAILURE);
		}

		sh->video_in.vid_buf[i].v4l2_buff_length = buffer.length;
		sh->video_in.vid_buf[i].index = i;
		sh->video_in.vid_buf[i].drm_buf.index = i;
		ret = drm_buffer_create(&v_pipe->drm,
				&sh->video_in.vid_buf[i].drm_buf,
				sh->video_in.format.width,
				sh->video_in.format.height,
				sh->video_in.format.bytesperline,
				sh->video_in.format.pixelformat);
		if (ret) {
			perror("unable to create capture buffers");
			exit(EXIT_FAILURE);
		}
		sh->video_in.vid_buf[i].dbuf_fd = sh->video_in.vid_buf[i].drm_buf.dbuf_fd;
		sh->video_in.vid_buf[i].v4l2_buff = sh->video_in.vid_buf[i].drm_buf.drm_buff;

		v4l2_queue_buffer(&sh->video_in, &sh->video_in.vid_buf[i]);
	}

	/* Start streaming */
	ret = v4l2_device_on(&sh->video_in);
	ASSERT2(ret >= 0, "v4l2_device_on [video_in] failed %d \n", ret);
	vlib_dbg("vlib :: Video Capture Pipeline started\n");

	/* TODO: Remove once image sensor subdevice driver becomes available */
	if (vdev->ops && vdev->ops->streamon) {
		ret = vdev->ops->streamon();
		ASSERT2(!ret, "streamon failed\n");
	}

	struct pollfd fds[] = {
		{.fd = sh->video_in.fd, .events = POLLIN},
	};

	/*
	 * NOTE: VDMA doesn't issue EOF interrupt, as a result even on the first frame done,
	 *       interrupt, it still updating it, current solution is to skip the first frame
	 *       done notification
	 */

	/* poll and pass buffers */
	sh->buffer_q_src2filter = g_queue_new();
	ASSERT2(sh->buffer_q_src2filter, "unable to create buffer queue\n");
	sh->buffer_q_filter2sink = g_queue_new();
	ASSERT2(sh->buffer_q_filter2sink, "unable to create buffer queue\n");
	sh->buffer_q_sink2src = g_queue_new();
	ASSERT2(sh->buffer_q_sink2src, "unable to create buffer queue\n");

	for (size_t i = 0; i < v_pipe->buffer_cnt; i++) {
		g_queue_push_head(sh->buffer_q_filter2sink,
				  &v_pipe->drm.d_buff[i]);
	}

	pthread_cleanup_push(m2m_sw_v4l2_cleanup, sh);

	while (poll(fds, ARRAY_SIZE(fds), POLL_TIMEOUT_MSEC) > 0 ) {
		if (fds[0].revents & POLLIN) {
			levents_capture_event(v_pipe->events[CAPTURE]);
			struct buffer *b = v4l2_dequeue_buffer(&sh->video_in,
							       sh->video_in.vid_buf);
			g_queue_push_head(sh->buffer_q_src2filter, b);

			int filter_has_func2 = !!sh->fs->ops->func2;
			if (g_queue_get_length(sh->buffer_q_src2filter) <
			    (M2M_SW_PIPELINE_DELAY_SRC2FILTER + 1 + filter_has_func2)) {
				continue;
			}

			b = g_queue_pop_tail(sh->buffer_q_src2filter);
			struct drm_buffer *b_out = g_queue_pop_tail(sh->buffer_q_filter2sink);
			levents_capture_event(v_pipe->events[PROCESS_IN]);
			unsigned short *out_ptr = (unsigned short *)b_out->drm_buff;
			unsigned short *in_ptr0 = (unsigned short *)b->v4l2_buff;
			if (filter_has_func2) {
				/* processing function takes two input frames */
				struct buffer *b2 = g_queue_peek_tail(sh->buffer_q_src2filter);
				unsigned short *in_ptr1 = (unsigned short *)b2->v4l2_buff;
				sh->fs->ops->func2(sh->fs, in_ptr1, in_ptr0, out_ptr,
						   sh->video_in.format.height,
						   sh->video_in.format.width,
						   sh->video_in.format.bytesperline,
						   sh->video_out.height,
						   sh->video_out.width,
						   sh->video_out.stride);
			} else {
				/* processing function takes one input frame */
				sh->fs->ops->func(sh->fs, in_ptr0, out_ptr,
						  sh->video_in.format.height,
						  sh->video_in.format.width,
						  sh->video_in.format.bytesperline,
						  sh->video_out.height,
						  sh->video_out.width,
						  sh->video_out.stride);
			}

			levents_capture_event(v_pipe->events[PROCESS_OUT]);

			/* queue used input buffer back at source */
			v4l2_queue_buffer(&sh->video_in, b);

			ret = drm_set_plane(&v_pipe->drm, b_out->index);
			if (ret < 0) {
				vlib_err("%s: flip failed\n", __func__);
				/* If the flip failed, requeue the buffer immediately */
				g_queue_push_head(sh->buffer_q_filter2sink, b_out);
			} else {
				/* If the flip succeeded, the previous buffer is now released.
				 * Requeue it on the V4L2 side, and store the index of the new
				 * buffer.
				 */
				if (!v_pipe->pflip_pending) {
					v_pipe->pflip_pending = 1;
					drm_wait_vblank(&v_pipe->drm, v_pipe);
				}

				g_queue_push_head(sh->buffer_q_sink2src, b_out);
				if (g_queue_get_length(sh->buffer_q_sink2src) >
				    M2M_SW_PIPELINE_DELAY_SINK2SRC + 1) {
					g_queue_push_head(sh->buffer_q_filter2sink,
							  g_queue_pop_tail(sh->buffer_q_sink2src));
				}
			}
		}
	}

	pthread_cleanup_pop(1);
}

static void m2m_sw_file_process_loop(struct video_pipeline *v_pipe,
				     const struct vlib_vdev *vdev,
				     const struct stream_handle *sh)
{
	int ret;
	size_t cur_drm_buf = 0;
	struct timespec sleep_time = {
		.tv_sec = 0,
		.tv_nsec = 1000000000 * v_pipe->fps.denominator / v_pipe->fps.numerator,
	};

	while (1) {
		uint8_t *in_buf = vdev->data.file.get_frame(vdev, v_pipe);
		ASSERT2(in_buf, "no input data\n");

		levents_capture_event(v_pipe->events[CAPTURE]);
		levents_capture_event(v_pipe->events[PROCESS_IN]);
		unsigned short *out_ptr = (unsigned short *)v_pipe->drm.d_buff[cur_drm_buf].drm_buff;
		unsigned short *in_ptr0 = (unsigned short *)in_buf;
		sh->fs->ops->func(sh->fs, in_ptr0, out_ptr,
				v_pipe->h, v_pipe->w, v_pipe->stride,
				sh->video_out.height,
				sh->video_out.width,
				sh->video_out.stride);

		levents_capture_event(v_pipe->events[PROCESS_OUT]);

		ret = drm_set_plane(&v_pipe->drm, cur_drm_buf);
		if (ret) {
			vlib_warn("buffer flip failed\n");
		}

		cur_drm_buf ^= 1;
		if (v_pipe->fps.denominator && v_pipe->fps.numerator) {
			while (nanosleep(&sleep_time, &sleep_time))
				;
		} else {
			pause();
		}
	}
}

/* Un-init m2m sw pipeline ->unmap buffers,stop the video stream and close video device*/
static void m2m_sw_pipeline_uninit(void *ptr)
{
	int ret;
	struct stream_handle *sh = ptr;
	struct video_pipeline *v_pipe = sh->vp;
	const struct vlib_vdev *vdev = v_pipe->vid_src;
	ASSERT2(vdev, "invalid video source\n");

	/* Set display to last buffer index */
	ret = drm_set_plane(&v_pipe->drm, v_pipe->buffer_cnt - 1);

	/* TODO: Remove once image sensor subdevice driver becomes available */
	if (vdev->ops && vdev->ops->streamoff) {
		ret = vdev->ops->streamoff();
		ASSERT2(!ret, "streamoff failed\n");
	}

	if (video_src_is_v4l2(vdev)) {
		ret = v4l2_device_off(&sh->video_in);
		ASSERT2(ret >= 0, "v4l2_device stream-off failed\n");
#ifdef WITH_SDSOC
		for (size_t i = 0; i < v_pipe->buffer_cnt; i++) {
			drm_buffer_destroy(v_pipe->drm.fd,
					   &sh->video_in.vid_buf[i].drm_buf);
		}
#endif
		v4l2_uninit(&sh->video_in);
	}

	if (vdev->ops && vdev->ops->unprepare) {
		ret = vdev->ops->unprepare(v_pipe, vdev);
	}

	if (v_pipe->enable_log_event) {
		levents_counter_stop(v_pipe->events[DISPLAY]);
		levents_counter_stop(v_pipe->events[CAPTURE]);
		levents_counter_stop(v_pipe->events[PROCESS_IN]);
		levents_counter_stop(v_pipe->events[PROCESS_OUT]);
	}

	free(sh);
}

/*
 * Process m2m sw pipeline input/output events.
 * TPG/External --> Software Processing -->DRM
 * Video input device allocate buffer - mmap that to userspace
 * DRM allocate framebuffer  -mmap that to userspace
 * Software processing logic uses these handles for reading/writing
 * raw/processed video frames.
 */
void *m2m_sw_process_event_loop(void *ptr)
{
	struct stream_handle *sh = ptr;
	struct video_pipeline *v_pipe = sh->vp;
	const struct vlib_vdev *vdev = v_pipe->vid_src;
	ASSERT2(vdev, "invalid video source\n");

	/* push cleanup handler */
	pthread_cleanup_push(m2m_sw_pipeline_uninit, ptr);
	if (v_pipe->enable_log_event) {
		levents_counter_start(v_pipe->events[DISPLAY]);
		levents_counter_start(v_pipe->events[CAPTURE]);
		levents_counter_start(v_pipe->events[PROCESS_IN]);
		levents_counter_start(v_pipe->events[PROCESS_OUT]);
	}

	if (video_src_is_v4l2(vdev)) {
		m2m_sw_v4l2_process_loop(sh);
	} else if (video_src_is_file(vdev)) {
		m2m_sw_file_process_loop(v_pipe, vdev, sh);
	}

	/* pop cleanup handler */
	pthread_cleanup_pop(1);

	pthread_exit(NULL);
}
