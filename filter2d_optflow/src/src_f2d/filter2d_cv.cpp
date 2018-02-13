/******************************************************************************
 *
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
 *
 *******************************************************************************/

/* Temporary fix for SDx Clang issue */
#ifdef __SDSCC__
#undef __ARM_NEON__
#undef __ARM_NEON
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#define __ARM_NEON__
#define __ARM_NEON
#else
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#endif

#include "filter2d_int.h"
#include <list>

using namespace cv;
using namespace std;
void filter2d_cv(unsigned short *frm_data_in, unsigned short *frm_data_out,
		 int height, int width, int stride, coeff_t coeff)
{
	Mat src(height, width, CV_8UC2, frm_data_in, stride);
	Mat dst(height, width, CV_8UC2, frm_data_out, stride);

	// planes
	std::vector<Mat> planes;

	// kernel
	Mat kernel = Mat(3, 3, CV_32SC1, (int *) coeff);

	// anchor
	Point anchor = Point(-1, -1);

	// filter
	split(src, planes);
	filter2D(planes[0], planes[0], -1, kernel, anchor, 0, BORDER_DEFAULT);
	merge(planes, dst);
}

#define RAW_IMAGE_CHANNEL	-1
#define RAW_IMAGE_SIZE		0
#define WEIGHT_CNT 			-1
#define CLASS_CNT			20

typedef struct the_weight
{
	unsigned short *weight;
	unsigned int len;
	unsigned int layer;
}THE_WEIGHT;

typedef struct image_data
{
	unsigned short *data;
	unsigned int len;
	unsigned int width;
	unsigned int height;
	unsigned int channel;
}IMAGE_DATA;


typedef struct infer_controller
{
	IMAGE_DATA *input;
	list<THE_WEIGHT> *weight_list;
	unsigned int cnt_weight;
}INFER_CONTROLLER;

INFER_CONTROLLER *alloc_infer_controller(IMAGE_DATA *input, list<THE_WEIGHT> &list)
{
	INFER_CONTROLLER *infer_tmp;
	return infer_tmp;
}

THE_WEIGHT *alloc_weight(unsigned short *weight, unsigned len)
{
	THE_WEIGHT *new_weight;
	return new_weight;
}

IMAGE_DATA *alloc_image_data(unsigned short *image, unsigned int len,
		unsigned width, unsigned int height, unsigned int channel)
{
	IMAGE_DATA *new_data;
	return new_data;
}

int get_the_weights(list<THE_WEIGHT> &list)
{
	return 0;
}

IMAGE_DATA *resized_image_data(IMAGE_DATA *ori_image)
{
	IMAGE_DATA *new_image;
	return new_image;
}

typedef struct the_predicted_data
{
	unsigned short *data;
	unsigned int w;
	unsigned int h;
	unsigned int d;
}THE_PREDICTED_DATA;

typedef struct the_box
{
	unsigned int x;
	unsigned int y;
	unsigned int w;
	unsigned int h;
	unsigned int confidence;
	unsigned int prob[CLASS_CNT];
}THE_BOX;

THE_PREDICTED_DATA *hw_cnn_network(INFER_CONTROLLER *infer)
{
	THE_PREDICTED_DATA *data;
	return data;
}

int post_process(THE_PREDICTED_DATA *data, list<THE_BOX> &box_list)
{
	return 0;
}

int boxes_projection(list<THE_BOX> &box_list)
{
	return 0;
}

int draw_the_boxes(unsigned short *frm_data_in, unsigned short *frm_data_out,
		 int height, int width, int stride, list<THE_BOX> box_list)
{
	return 0;
}
int object_detection(unsigned short *frm_data_in, unsigned short *frm_data_out,
		 int height, int width, int stride)
{
	unsigned short *tmp;
	int ret = 0;

	list<THE_WEIGHT> weights_list;
	list <THE_BOX> drawing_box_list;
	IMAGE_DATA *ori_image;
	IMAGE_DATA *new_image;
	INFER_CONTROLLER *infer_controller;
	THE_PREDICTED_DATA *the_predicted_data;

	ori_image = alloc_image_data(frm_data_in, (width*height), width, height, RAW_IMAGE_CHANNEL);
	new_image = resized_image_data(ori_image);
	ret = get_the_weights(weights_list);
	infer_controller = alloc_infer_controller(new_image, weights_list);
	the_predicted_data = hw_cnn_network(infer_controller);
	ret |= post_process(the_predicted_data, drawing_box_list);
	ret |= boxes_projection(drawing_box_list);
	ret |= draw_the_boxes(frm_data_in, frm_data_out, height, width, stride, drawing_box_list);

	if(ret)
		return -1;
	else
		return 0;
}


void draw_the_box(unsigned short *frm_data_in, unsigned short *frm_data_out,
		 int height, int width, int stride)
{
	Mat src(height, width, CV_8UC2, frm_data_in, stride);
	Mat dst(height, width, CV_8UC2, frm_data_out, stride);
	// planes
	std::vector<Mat> planes;
	// filter
	split(src, planes);
	rectangle(planes[0], Point(400,500), Point(600,600),Scalar(175),2,8,0);
	merge(planes, dst);
	//printf("[Lucas] %dX%d drawing complete...\n", height, width);
	/* FIXME Need to check why only output half image. and double rectangle
	 * rectangle(src, Point(0,0), Point(200,100),Scalar(255),2,8,0);
	 * src.copyTo(dst);
	 * */
}
