/*
 * hash_chain.hpp
 *
 *  Created on: Feb 27, 2018
 *      Author: lucas
 */

#ifndef HASH_CHAIN_HPP_
#define HASH_CHAIN_HPP_

#include <list>
#include "object_dector.hpp"
#include <string>

typedef struct chain_mgr
{
	int nrm;
	int size;
	std::list<THE_BOX> *hash_table;
}CHAIN_MGR;

class hash_chain{

private:
	CHAIN_MGR mgr;

public:
	hash_chain(int nrm, int size);
	~hash_chain();
	void insert(THE_BOX *the_box);
	void extract(std::list<THE_BOX> &out_list);
};


#endif /* HASH_CHAIN_HPP_ */
