//
//	Compressor.h
//	LogCompression
//	
//	Created by Chongbo Wei on 2019/4/12
//	Copyright © 2019 Chongbo Wei. All rights reserved.

#ifndef COMPRESSOR_H_
#define COMPRESSOR_H_

#include "base.h"
#include "Logs.h"

class Compressor {
public:
	static void Compress(OneAttributeList *oneAttributeList, HashTable *hashTable, std::list<uint32_t>&logs,std::vector<std::list<uint32_t> > &indexs) {
		char **List = oneAttributeList->getList();
		uint32_t precode = -1;
		uint32_t hashID;
		for(uint32_t i=0; i<oneAttributeList->getListLen(); i++) {
			hashID = hashTable->insert(List[i]);
			if(hashID == indexs.size()) indexs.push_back(std::list<uint32_t>());
			if(precode != hashID) {
				if(precode != -1) {
					logs.push_back(precode);
					logs.push_back(i-1);
					indexs[precode].push_back(i-1);
				}
				indexs[hashID].push_back(i);
				precode = hashID;
			}
		}
		if(precode != -1)
			indexs[precode].push_back(oneAttributeList->getListLen()-1);
	}
};



#endif /* COMPRESSOR_H_ */
