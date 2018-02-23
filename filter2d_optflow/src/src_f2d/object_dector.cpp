
#ifdef __SDSCC__
#undef __ARM_NEON__
#undef __ARM_NEON
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#define __ARM_NEON__
#define __ARM_NEON
#else
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#endif

#include "object_dector.hpp"
#include "object_int.h"
using namespace cv;
using namespace std;



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

#define INPUT_RAW_PIXEL (416)
#define INPUT_COL_PIXEL (416)
#define INPUT_SIZE  (INPUT_RAW_PIXEL*INPUT_COL_PIXEL)
#define INPUT_RGB_CHANNEL 3
#define OUTPUT_RAW_PIXEL		INPUT_RAW_PIXEL
#define OUTUT_COL_PIXEL		INPUT_COL_PIXEL
#define OUTPUT_LOCATION_X	(400)
#define OUTPUT_LOCATION_Y	(500)


void rgb2yuv422(Mat *src, Mat *dst)
{
#if 1
	int i,j,n_row,n_col;
	int R, G, B;
	int Y, U, V;
	int RGBIndex, YIndex, UVIndex;
	unsigned char *RGBBuffer = src->ptr(0);
	unsigned char *yuyvBuffer;
	unsigned char *YBuffer = new unsigned char[INPUT_SIZE];
	unsigned char *UBuffer = new unsigned char[INPUT_SIZE/2];
	unsigned char *VBuffer = new unsigned char[INPUT_SIZE/2];
	unsigned char *ULine = (new unsigned char[INPUT_RAW_PIXEL+2])+1;
	unsigned char *VLine = (new unsigned char[INPUT_RAW_PIXEL+2])+1;

	ULine[-1]=ULine[512]=128;
	VLine[-1]=VLine[512]=128;

	for (i=0; i<INPUT_COL_PIXEL; i++)
	{
		RGBIndex = 3*INPUT_RAW_PIXEL*i;
		YIndex    = INPUT_RAW_PIXEL*i;
		UVIndex   = INPUT_RAW_PIXEL*i/2;

		for ( j=0; j<INPUT_RAW_PIXEL; j++)
		{
			R = RGBBuffer[RGBIndex++];
			G = RGBBuffer[RGBIndex++];
			B = RGBBuffer[RGBIndex++];
			//Convert RGB to YUV
			Y = (unsigned char)( ( 66 * R + 129 * G +   25 * B + 128) >> 8) + 16   ;
			U = (unsigned char)( ( -38 * R -   74 * G + 112 * B + 128) >> 8) + 128 ;
			V = (unsigned char)( ( 112 * R -   94 * G -   18 * B + 128) >> 8) + 128 ;
			YBuffer[YIndex++] = static_cast<unsigned char>( (Y<0) ? 0 : ((Y>255) ? 255 : Y) );
			VLine[j] = V;
			ULine[j] = U;
		}
		for ( j=0; j<INPUT_RAW_PIXEL; j+=2)
		{
			//Filter line
			V = ((VLine[j-1]+2*VLine[j]+VLine[j+1]+2)>>2);
			U = ((ULine[j-1]+2*ULine[j]+ULine[j+1]+2)>>2);

			//Clip and copy UV to output buffer
			VBuffer[UVIndex] = static_cast<unsigned char>( (V<0) ? 0 : ((V>255) ? 255 : V) );
			UBuffer[UVIndex++] = static_cast<unsigned char>( (U<0) ? 0 : ((U>255) ? 255 : U) );
		}
	}
	YIndex = 0;
	UVIndex = 0;

	for (n_row = 0; n_row < OUTUT_COL_PIXEL; n_row++)
	{
		yuyvBuffer = dst->ptr(OUTPUT_LOCATION_X+n_row, OUTPUT_LOCATION_Y);//(row(h),col(p))

		for(n_col = 0; n_col < OUTPUT_RAW_PIXEL; n_col+=2)
		{
			yuyvBuffer[n_col*2 + 0] = YBuffer[YIndex++];
			yuyvBuffer[n_col*2 + 1] = UBuffer[UVIndex];
			yuyvBuffer[n_col*2 + 2] = YBuffer[YIndex++];
			yuyvBuffer[n_col*2 + 3] = VBuffer[UVIndex++];
		}
	}
#if 0
	delete [] YBuffer;
	delete [] UBuffer;
	delete [] VBuffer;
	delete [] ULine;
	delete [] VLine;
#endif
	return;

#endif
}

static int video_mode_processing(unsigned short *frm_data_in, unsigned short *frm_data_out,
		 int height, int width, int stride)
{
	printf("[OBJ_DECTETOR] Pure post processing mode\n...");
	Mat image;
	Mat rgb_image;
	Mat dst(height, width, CV_8UC2, frm_data_out, stride);

	image = imread("/media/card/dog416.jpg",CV_LOAD_IMAGE_COLOR);
    if(! image.data )
    {
           printf("[OBJ_DECTOR] Could not open or find the image\n");
           return -1;
    }

	cvtColor(image, rgb_image, COLOR_BGR2RGB);
	rgb2yuv422(&rgb_image, &dst);

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

#if (DEMO_MODE)
	ori_image = alloc_image_data(frm_data_in, (width*height), width, height, RAW_IMAGE_CHANNEL);
	new_image = resized_image_data(ori_image);
	ret = get_the_weights(weights_list);
	infer_controller = alloc_infer_controller(new_image, weights_list);
	the_predicted_data = hw_cnn_network(infer_controller);
	ret |= post_process(the_predicted_data, drawing_box_list);
	ret |= boxes_projection(drawing_box_list);
#elif (PURE_POST_PROCESSING)
	ret |= video_mode_processing(frm_data_in, frm_data_out, height, width, stride);
#else
	ret |= draw_the_boxes(frm_data_in, frm_data_out, height, width, stride, drawing_box_list);
#endif

	if(ret)
		return -1;
	else
		return 0;
}


static void rgb2yuyv(Mat rgb_image, Mat yuyv_image)
{

	return;
}


static void draw_the_box(unsigned short *frm_data_in, unsigned short *frm_data_out,
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













