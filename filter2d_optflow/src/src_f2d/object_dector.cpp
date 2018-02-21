
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

#include <list>
#include "object_dector.hpp"
using namespace cv;
using namespace std;

typedef struct infer_controller
{
	IMAGE_DATA *input;
	std::list<THE_WEIGHT> *weight_list;
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

static int get_the_weights(list<THE_WEIGHT> &list)
{
	return 0;
}

static IMAGE_DATA *resized_image_data(IMAGE_DATA *ori_image)
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

static THE_PREDICTED_DATA *hw_cnn_network(INFER_CONTROLLER *infer)
{
	THE_PREDICTED_DATA *data;
	return data;
}

static int post_process(THE_PREDICTED_DATA *data, list<THE_BOX> &box_list)
{
	return 0;
}

static int boxes_projection(list<THE_BOX> &box_list)
{
	return 0;
}

static int draw_the_boxes(unsigned short *frm_data_in, unsigned short *frm_data_out,
		 int height, int width, int stride, list<THE_BOX> box_list)
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
#if (!BOX_DRAWING_MODE)
	ori_image = alloc_image_data(frm_data_in, (width*height), width, height, RAW_IMAGE_CHANNEL);
	new_image = resized_image_data(ori_image);
	ret = get_the_weights(weights_list);
	infer_controller = alloc_infer_controller(new_image, weights_list);
	the_predicted_data = hw_cnn_network(infer_controller);
	ret |= post_process(the_predicted_data, drawing_box_list);
	ret |= boxes_projection(drawing_box_list);
#else
	ret |= draw_the_boxes(frm_data_in, frm_data_out, height, width, stride, drawing_box_list);
#endif

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













