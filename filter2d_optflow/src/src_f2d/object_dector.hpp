#ifndef SRC_SRC_F2D_OBJECT_DECTOR_HPP_
#define SRC_SRC_F2D_OBJECT_DECTOR_HPP_

#include <list>
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

typedef struct infer_controller
{
	IMAGE_DATA *input;
	std::list<THE_WEIGHT> *weight_list;
	unsigned int cnt_weight;
}INFER_CONTROLLER;


#endif /* SRC_SRC_F2D_OBJECT_DECTOR_HPP_ */
