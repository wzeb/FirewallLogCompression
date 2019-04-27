//
//	CompressedLogs.cpp
//	LogCompression
//	
//	Created by Chongbo Wei on 2019/4/12
//	Copyright © 2019 Chongbo Wei. All rights reserved.

#ifndef SRC_COMPRESSEDLOGS_H_
#define SRC_COMPRESSEDLOGS_H_

#include "base.h"
#include "IOBuffer.h"

class CompressedLogs {
private:
	std::list<uint32_t> logs;
	uint32_t preCode;
	uint32_t preCount;
	uint32_t maxCode;
public:
	CompressedLogs():preCode(-1), preCount(-1), maxCode(0){}

	void AddCompressedLog(const uint32_t &hashCode) {
		if(preCode != hashCode) {
			if(preCode != -1) {
				logs.push_back(preCode);
				logs.push_back(preCount);
			}
			preCode = hashCode;
		}
		preCount++;
		if(hashCode > maxCode) maxCode = hashCode;
	}

	uint64_t GetDiskConsumption() {
		if(!preCount) return 0;
		uint32_t codeLen = 32;
		uint32_t lineLen = 32;
		while(!((this->maxCode|1)&(1<<codeLen-1))) codeLen--;
		while(!((this->preCount|1)&(1<<lineLen-1))) lineLen--;
		return (uint64_t)(codeLen+lineLen)*logs.size()/2/8+8;
	}

	void DumpToFile(const char *output_path) {
		logs.push_back(preCode);
		logs.push_back(preCount);
		uint32_t bufferLen = std::min((uint64_t)10240000, (uint64_t)CONFIGURATION.getMemoryLimit()*1024*1024/5/4);
		IOBuffer<uint32_t> *buffer_out = new IOBuffer<uint32_t>(output_path, "ab+", bufferLen);
		uint32_t codeLen = 32;
		uint32_t lineLen = 32;
		while(!((this->maxCode|1)&(1<<codeLen-1))) codeLen--;
		while(!((this->preCount|1)&(1<<lineLen-1))) lineLen--;
		buffer_out->writeCompressedLogs(logs, codeLen, lineLen);
		printf("writed %s\n", output_path);
		delete buffer_out;
	}

};


#endif /* SRC_COMPRESSEDLOGS_H_ */
