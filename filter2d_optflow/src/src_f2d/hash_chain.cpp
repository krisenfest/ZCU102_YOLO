/*
 * hash_chain.cpp
 *
 *  Created on: Feb 27, 2018
 *      Author: lucas
 */
#include "hash_chain.hpp"
#include <stdio.h>
using namespace std;
extern char *class_names[];


static float overlap(float x1, float w1, float x2, float w2)
{
    float l1 = x1 - w1/2;
    float l2 = x2 - w2/2;
    float left = l1 > l2 ? l1 : l2;
    float r1 = x1 + w1/2;
    float r2 = x2 + w2/2;
    float right = r1 < r2 ? r1 : r2;
    return right - left;
}

static float box_intersection(THE_BOX &a, THE_BOX &b)
{
    float w = overlap(a.x, a.w, b.x, b.w);
    float h = overlap(a.y, a.h, b.y, b.h);
    if(w < 0 || h < 0) return 0;
    float area = w*h;
    return area;
}

static float box_union(THE_BOX &a, THE_BOX &b)
{
    float i = box_intersection(a, b);
    float u = a.w*a.h + b.w*b.h - i;
    return u;
}

float box_iou(THE_BOX &a, THE_BOX &b)
{
    return box_intersection(a, b)/box_union(a, b);
}


hash_chain::hash_chain(int nrm, int size)
{
	this->mgr.nrm = nrm;
	this->mgr.size= size;
	this->mgr.hash_table = new list<THE_BOX> [size]();
}

hash_chain::~hash_chain()
{
	delete this->mgr.hash_table;
}
void hash_chain::insert(THE_BOX *the_box)
{
	int index = the_box->candidate->index;
	list <THE_BOX> *class_list;
	class_list = &(this->mgr.hash_table[index]);
#if 1
	for(list<THE_BOX>::iterator it = class_list->begin(); it != class_list->end(); ++it)
	{
		if(box_iou(*the_box, *it) > NRM_THRESH)
		{
			if (the_box->score > it->score)
			{
				class_list->erase(it);
				it--;
			}
			else
				return; /*the better candidate is existed, return directly*/
		}
	}
#endif
	class_list->push_front(*the_box);
}

void hash_chain::extract(list<THE_BOX> &out_list)
{
	list <THE_BOX> *class_ptr;
	for(int i = 0; i < this->mgr.size; i++ )
	{
		class_ptr = &this->mgr.hash_table[i];
		if( class_ptr->empty())
			continue;
		else
		{
			 while (!class_ptr->empty())
			 {
				 THE_BOX &b = class_ptr->front();
				 dector_printf("prob=%f, class=%s(%d), x=%f,y=%f,w=%f,h=%f\n",
						 b.score,
						 class_names[b.candidate->index],
						 b.candidate->index,
						 b.x, b.y, b.w, b.h);

				 out_list.push_front(b);
				 class_ptr->pop_front();
			  }
		}
	}
	return;
}
