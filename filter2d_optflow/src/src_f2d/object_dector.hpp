#ifndef SRC_SRC_F2D_OBJECT_DECTOR_HPP_
#define SRC_SRC_F2D_OBJECT_DECTOR_HPP_

#define RAW_IMAGE_CHANNEL	-1
#define RAW_IMAGE_SIZE		0
#define WEIGHT_CNT 			-1
#define CLASS_CNT			20
#define BOX_DRAWING_MODE	1

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

int object_detection(unsigned short *frm_data_in, unsigned short *frm_data_out,
		 int height, int width, int stride);

void draw_the_box(unsigned short *frm_data_in, unsigned short *frm_data_out,
		 int height, int width, int stride);

#endif /* SRC_SRC_F2D_OBJECT_DECTOR_HPP_ */
