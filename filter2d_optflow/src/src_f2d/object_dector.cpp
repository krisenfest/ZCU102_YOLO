#if (SIMULATE)
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#else
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
#endif  //__SDSCC__
#endif//SIMULATE

#include "object_dector.hpp"
#include "object_int.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "hash_chain.hpp"
#include <cmath>
#include <cassert>
using namespace cv;
using namespace std;
static float colors[6][3] = { {1,0,1}, {0,0,1},{0,1,1},{0,1,0},{1,1,0},{1,0,0} };
char *class_names[20] = {"aeroplane", "bicycle", "bird", "boat", "bottle", "bus", "car", "cat", "chair", "cow", "diningtable", "dog", "horse", "motorbike", "person", "pottedplant", "sheep", "sofa", "train", "tvmonitor"};

static void rgb2yuv422(Mat *src, Mat *dst);

static int draw_the_boxes(unsigned short *frm_data_in, unsigned short *frm_data_out,
		 int height, int width, int stride, list<THE_BOX> &box_list);

static int draw_the_boxes_sim(Mat &src, list<THE_BOX> &box_list);

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


static THE_PREDICTED_DATA *hw_cnn_network(INFER_CONTROLLER *infer)
{


#if (PURE_POST_PROCESSING)
	int fd;
	int nr;
	THE_PREDICTED_DATA *predict_data = new THE_PREDICTED_DATA();
#endif

#if (PURE_POST_PROCESSING)

	unsigned char *raw_data = new unsigned char[PREDICT_CUBE_SIZE];
	predict_data->data = (RAW_DATA_LAYOUT *)raw_data;
	predict_data->w = PREDICT_CUBE_WIDTH;
	predict_data->h = PREDICT_CUBE_HEIGHT;
	predict_data->d = PREDICT_CUBE_DEPTH;

#if SIMULATE
	fd = open("../src/resource/prediction_cube.bin", O_RDONLY, S_IRUSR | S_IWUSR);
#else
	fd = open("/media/card/prediction_cube.bin", O_RDONLY, S_IRUSR | S_IWUSR);
#endif

	nr = read(fd,predict_data->data, PREDICT_CUBE_SIZE);
	if (nr == -1)
		dector_printf("predict_cube read error: %s\n", strerror(errno));
#endif

	close(fd);
	return predict_data;
}

static void dump_the_box(THE_BOX &b)
{
	dector_printf("score=%f, x=%f, y=%f, w=%f, h=%f\n",
			b.score,
			b.x,
			b.y,
			b.w,
			b.h);
}

static int post_process(THE_PREDICTED_DATA *predict_data, list<THE_BOX> &box_list)
{

	int g,b,cl;
	hash_chain *map = new hash_chain(1,CLASS_CNT);

	for (g = 0; g < PREDICT_NUM_OF_GRIDS; g++)
	{
		int r = g / PREDICT_CUBE_WIDTH;
		int c = g % PREDICT_CUBE_WIDTH;
		THE_CLASS *the_cdc = new THE_CLASS();
		float *class_prob = new float[CLASS_CNT];

		for(cl = 0; cl < CLASS_CNT; cl++)
		{
			float prob;
			prob = predict_data->data->class_prob[r][c][cl];
			class_prob[cl] = prob;
			if(prob > the_cdc->prob)
			{
				the_cdc->index = cl;
				the_cdc->prob = prob;
			}
		}

		if(the_cdc->prob == 0)
		{
			delete the_cdc;
			delete class_prob;
			continue;
		}

		the_cdc->class_prob = class_prob;

		for (b = 0 ; b < PREDICT_NUM_OF_BOX; b++)
		{
			THE_BOX *the_box = new THE_BOX;
			the_box->candidate = the_cdc;
			the_box->x = (predict_data->data->loc[r][c][b].x + c)/PREDICT_CUBE_WIDTH * PREDICT_CUBE_CENTER_W_SCALE;
			the_box->y = (predict_data->data->loc[r][c][b].y + r)/PREDICT_CUBE_WIDTH * PREDICT_CUBE_CENTER_H_SCALE;
			the_box->w = pow(predict_data->data->loc[r][c][b].w, (PREDICT_CUBE_SIZE_SQUARE ? 2:1)* PREDICT_CUBE_CENTER_W_SCALE);
			the_box->h = pow(predict_data->data->loc[r][c][b].h, (PREDICT_CUBE_SIZE_SQUARE ? 2:1)* PREDICT_CUBE_CENTER_H_SCALE);
			the_box->con = predict_data->data->con[r][c][b];
			the_box->score = the_box->con * the_box->candidate->prob;

			if(the_box->score > BOX_THRESH)
			{
				dump_the_box(*the_box);
				map->insert(the_box);
			}
		}
	}

	map->extract(box_list);
	return 0;
}

static int boxes_projection(list<THE_BOX> &box_list)
{
	return 0;
}


static int video_mode_processing(unsigned short *frm_data_in, unsigned short *frm_data_out,
		 int height, int width, int stride)
{



	dector_printf("Pure post processing mode...\n");
	Mat image;
	Mat rgb_image;
	THE_PREDICTED_DATA *predict_data;
	list<THE_BOX> drawing_box_list;

#if (SIMULATE)
	image = imread("../src/resource/dog.jpg",CV_LOAD_IMAGE_COLOR);
#else
	//image = imread("/media/card/dog416.jpg",CV_LOAD_IMAGE_COLOR);
	image = imread("/media/card/dog.jpg",CV_LOAD_IMAGE_COLOR);
#endif
	if(! image.data )
    {
           dector_printf("Could not open or find the image\n");
           return -1;
    }
#if (SIMULATE)
	predict_data = hw_cnn_network(NULL);
	post_process(predict_data, drawing_box_list);
	draw_the_boxes_sim(image, drawing_box_list);

#else
	predict_data = hw_cnn_network(NULL);
	post_process(predict_data, drawing_box_list);
	draw_the_boxes_sim(image, drawing_box_list);
	cvtColor(image, rgb_image, COLOR_BGR2RGB);
	Mat dst(height, width, CV_8UC2, frm_data_out, stride);
	rgb2yuv422(&rgb_image, &dst);
#endif

	image.release();
	rgb_image.release();

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




float get_color(int c, int x, int max)
{
    float ratio = ((float)x/max)*5;
    int i = floor(ratio);
    int j = ceil(ratio);
    ratio -= i;
    float r = (1-ratio) * colors[i][c] + ratio*colors[j][c];
    return r;
}


static void draw_the_box(Mat &p, THE_BOX &b)
{
	int left = (b.x-b.w/2.)*INPUT_RAW_PIXEL;
	int right = (b.x+b.w/2.)*INPUT_RAW_PIXEL;
    int top   = (b.y-b.h/2.)*INPUT_COL_PIXEL;
    int bot   = (b.y+b.h/2.)*INPUT_COL_PIXEL;

    int offset = b.candidate->index *123457 % CLASS_CNT;
    float red = get_color(2,offset,CLASS_CNT);
    float green = get_color(1,offset,CLASS_CNT);
    float blue = get_color(0,offset,CLASS_CNT);
    float bgr[3];

    bgr[0] = blue * 100;
    bgr[1] = green * 100;
    bgr[2] = red * 100;

    dector_printf("color = (%f, %f, %f)\n", bgr[0], bgr[1], bgr[2]);
	rectangle(p, Point(left,top), Point(right,bot),Scalar(bgr[0],bgr[1],bgr[2]),2,8,0);
	putText(p, class_names[b.candidate->index], Point(left,top-4),FONT_HERSHEY_DUPLEX,0.7, Scalar(bgr[0],bgr[1],bgr[2]), 2, 2, false);
}

static int draw_the_boxes_sim(Mat &src, list<THE_BOX> &box_list)
{
	/*draw the box iteratively*/
	for(list<THE_BOX>::iterator it = box_list.begin(); it != box_list.end(); ++it)
	{
		draw_the_box(src, *it);
	}

#if (SIMULATE)
	namedWindow( "Display Image", CV_WINDOW_AUTOSIZE );
	imshow( "Display Image", src );
	waitKey(0);
#endif
	return 0;
}


static int draw_the_boxes(unsigned short *frm_data_in, unsigned short *frm_data_out,
		 int height, int width, int stride, list<THE_BOX> &box_list)
{

	assert(frm_data_in  != NULL);
	assert(frm_data_out != NULL);
	Mat src(height, width, CV_8UC2, frm_data_in, stride);
	Mat dst(height, width, CV_8UC2, frm_data_out, stride);

	std::vector<Mat> planes;
	// get one of RGB plane to draw
	split(src, planes);

	/*draw the box iteratively*/
	for(list<THE_BOX>::iterator it = box_list.begin(); it != box_list.end(); ++it)
	{
		draw_the_box(planes[0], *it);
	}

	merge(planes, dst);
	return 0;
}



static void rgb2yuv422(Mat *src, Mat *dst)
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
		yuyvBuffer = dst->ptr(OUTPUT_LOCATION_X+2*n_row, OUTPUT_LOCATION_Y);//(row(h),col(p))

		for(n_col = 0; n_col < OUTPUT_RAW_PIXEL; n_col+=2)
		{
			yuyvBuffer[n_col*2 + 0] = YBuffer[YIndex++];
			yuyvBuffer[n_col*2 + 1] = UBuffer[UVIndex];
			yuyvBuffer[n_col*2 + 2] = YBuffer[YIndex++];
			yuyvBuffer[n_col*2 + 3] = VBuffer[UVIndex++];
		}
	}
	delete [] YBuffer;
	delete [] UBuffer;
	delete [] VBuffer;
	delete [] (ULine-1);
	delete [] (VLine-1);

	return;

#endif
}









