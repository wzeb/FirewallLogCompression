//
//	Indexs.h
//	LogCompression
//	
//	Created by Chongbo Wei on 2019/4/12
//	Copyright © 2019 Chongbo Wei. All rights reserved.

#ifndef SRC_INDEXS_H_
#define SRC_INDEXS_H_

#include "base.h"
#include "IOBuffer.h"

class Indexs {
private:
	std::vector<std::list<uint32_t> > indexs;
	uint32_t preCode;
	uint32_t maxLine;

public:
	Indexs():preCode(-1), maxLine(0) {}
	void AddIndex(uint32_t hashCode, uint32_t lineID) {
		if(maxLine < lineID) maxLine = lineID;
		if(hashCode == preCode) return;
		if(hashCode == indexs.size()) indexs.push_back(std::list<uint32_t>());
		if(preCode != -1) indexs[preCode].push_back(lineID-1);
		indexs[hashCode].push_back(lineID);
		preCode = hashCode;
	}

	void DumpToFile(const char *output_path) {
		if(preCode != -1) indexs[preCode].push_back(maxLine);
		uint32_t bufferLen = std::min((uint64_t)10240000, (uint64_t)CONFIGURATION.getMemoryLimit()*1024*1024/5/4);
		IOBuffer<uint32_t> *buffer_out = new IOBuffer<uint32_t>(output_path, "ab+", bufferLen);
		uint32_t maxLine = 0;
		uint32_t lineLen = 32;
		for(std::vector<std::list<uint32_t> >::iterator it=indexs.begin(); it!=indexs.end(); it++) {
			if(maxLine < it->back()) maxLine = it->back();
		}
		while(!(maxLine&(1<<lineLen-1))) lineLen--;
		buffer_out->DumpIndexsSet(indexs, lineLen, indexs.size());
		printf("writed %s\n", output_path);
		delete buffer_out;
	}

	uint64_t GetDiskConsumption() {
		uint64_t ans = 8;
		ans += 8 * indexs.size();
		uint32_t lineLen = 32;
		uint64_t count = 0;
		for(uint32_t i=0; i<indexs.size(); i++) {
			std::list<uint32_t>& lst = indexs[i];
			count += lst.size();
		}
		while(!(this->maxLine&(1<<lineLen-1))) lineLen--;
		ans += count * lineLen / 8;
		return ans;
	}
};



#endif /* SRC_INDEXS_H_ */
