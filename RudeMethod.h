//
//	Retrieval.h
//	LogCompression
//
//	Created by Chongbo Wei on 2019/4/12
//	Copyright © 2019 Chongbo Wei. All rights reserved.

#ifndef RUDEMETHOD_H_
#define RUDEMETHOD_H_

#include "base.h"
#include "config_parser.h"
#include "IOBuffer.h"

class RudeMethod {
public:
	static std::list<char**>  Search(const uint32_t &cID, const char *value, uint32_t &cnt);
};

std::list<char**>  RudeMethod::Search(const uint32_t &cID, const char *value, uint32_t &cnt) {
	std::list<char**> rt;
	uint32_t attributesNum = CONFIGURATION.getAttributesNum();
	uint32_t bufferLen = std::min((uint64_t)10240000, (uint64_t)CONFIGURATION.getMemoryLimit()*1024*1024/10);
	IOBuffer<char> *buffer_in = new IOBuffer<char>(CONFIGURATION.getFilePath().c_str(), "r", bufferLen);
	char **item = new char*[attributesNum];
	while (buffer_in->getNextItem(item)) {
		if(strcmp(item[cID], value) == 0) {
			rt.push_back(item);
			for(int i=0; i<attributesNum-1; i++) {
				printf("%s,", item[i]);
			}
			printf("%s\n", item[attributesNum-1]);
		}
	}
	cnt = rt.size();
	delete buffer_in;
	return rt;
}




#endif /* RUDEMETHOD_H_ */
