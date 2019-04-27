//
//	Retrieval.h
//	LogCompression
//	
//	Created by Chongbo Wei on 2019/4/12
//	Copyright © 2019 Chongbo Wei. All rights reserved.

#ifndef SRC_RETRIEVAL_H_
#define SRC_RETRIEVAL_H_

#include "base.h"
#include "config_parser.h"
#include "IOBuffer.h"
#include "Compressor.h"
#include "CompressedLogs.h"
#include "Indexs.h"

class Retrieval{
public:
	static uint32_t ** Search(const uint32_t &cID, const char *value, uint32_t &cnt);
	static uint32_t GetCodeByStrValue(const uint32_t &cID, const char *value);
	static uint32_t GetCodeByInt16Value(const uint32_t &cID, const uint32_t &value);
	static uint32_t GetCodeByInt32Value(const uint32_t &cID, const uint32_t &value);
	static std::list<Interval>* GetSetsByCode(const uint32_t &cID, const uint32_t &code);
	static void GetItemsBySets(uint32_t **items, const uint32_t&cnt, std::list<Interval> *setList);
	static void DisplayResults(uint32_t **items, const uint32_t &cnt);
};

uint32_t Retrieval::GetCodeByStrValue(const uint32_t &cID, const char *value) {
	char fileName[] = "./out/hash/hash-00";
	fileName[strlen(fileName)-2] = '0' + cID/10;
	fileName[strlen(fileName)-1] = '0' + cID%10;
	uint32_t bufferLen = std::min((uint64_t)1024000, (uint64_t)CONFIGURATION.getMemoryLimit()*1024*1024/5);
	IOBuffer<char> *buffer_in = new IOBuffer<char>(fileName, "r", bufferLen);
	uint32_t code = 0;
	char *line = NULL;
	while(buffer_in->getNextLine(line)) {
		//assert(line != NULL);
		if(strcmp(line, value) == 0) {
			delete buffer_in;
			return code;
		}
		code ++;
	}
	delete buffer_in;
	return -1;
}

uint32_t Retrieval::GetCodeByInt16Value(const uint32_t &cID, const uint32_t &value) {
	char fileName[] = "./out/hash/hash-00";
	fileName[strlen(fileName)-2] = '0' + cID/10;
	fileName[strlen(fileName)-1] = '0' + cID%10;
	//printf("fopen : %s\n", fileName);
	uint32_t bufferLen = std::min((uint64_t)1024000, (uint64_t)CONFIGURATION.getMemoryLimit()*1024*1024/10);
	IOBuffer<uint16_t> *buffer_in = new IOBuffer<uint16_t>(fileName, "rb", bufferLen);
	uint32_t code = 0;
	uint32_t x = 0;
	while(buffer_in->getNextInt(x)) {
		if(x == value) {
			delete buffer_in;
			return code;
		}
		code ++;
	}
	delete buffer_in;
	return -1;
}

uint32_t Retrieval::GetCodeByInt32Value(const uint32_t &cID, const uint32_t &value) {
	char fileName[] = "./out/hash/hash-00";
	fileName[strlen(fileName)-2] = '0' + cID/10;
	fileName[strlen(fileName)-1] = '0' + cID%10;
	uint32_t bufferLen = std::min((uint64_t)1024000, (uint64_t)CONFIGURATION.getMemoryLimit()*1024*1024/10);
	IOBuffer<uint32_t> *buffer_in = new IOBuffer<uint32_t>(fileName, "rb", bufferLen);
	uint32_t code = 0;
	uint32_t x = 0;
	while(buffer_in->getNextInt(x)) {
		//printf(" get %u  ", x);
		if(x == value) {
			delete buffer_in;
			return code;
		}
		code ++;
	}
	delete buffer_in;
	return -1;
}


/**
 * pFile : opened binary sets file, remember to close
 * offset_s : left bound -- bit
 * offset_e : right bound -- bit
 * codeLen : bit length for a number
 */
std::list<Interval> *GetSetsFromFile(FILE *pFile, const uint64_t &offset_s,
		const uint64_t &offset_e, const uint32_t &codeLen) {
	std::list<Interval> *setList = new std::list<Interval>();
	uint32_t start = offset_s/32;
	uint32_t offset = offset_s%32;
	uint32_t end = offset_e/32;
	uint32_t count = end-start+1;
	uint32_t *buffer = new uint32_t[count];
	uint32_t readNum = 0;
	uint32_t idx = 0;
	fseek(pFile, start<<2 , SEEK_SET);
	while(count) {
		readNum = fread(buffer+idx, sizeof(uint32_t), count, pFile);
		count -= readNum;
		idx += readNum;
	}
	count = (offset_e - offset_s) / codeLen;
	idx = 0;
	while(count) {
        Interval itv(0,0);
		if(get_from_bit(buffer+idx, offset, itv.L, codeLen)) idx++;
        if(get_from_bit(buffer+idx, offset, itv.R, codeLen)) idx++;
		setList->push_back(itv);
        count -= 2;
	}
	delete []buffer;
	return setList;
}

std::list<Interval>* Retrieval::GetSetsByCode(const uint32_t &cID, const uint32_t &code) {
	char fileName[] = "./out/indexs/index-00";
	fileName[strlen(fileName)-2] = '0' + cID/10;
	fileName[strlen(fileName)-1] = '0' + cID%10;
	FILE *pFile = fopen(fileName, "rb");
	uint32_t codeNum = 0;
	uint32_t codeLen = 0;
	uint64_t offset_s = 0;
	uint64_t offset_e = 1<<63;
	fread(&codeNum, sizeof(uint32_t), 1, pFile);
	fread(&codeLen, sizeof(uint32_t), 1, pFile);
	fseek(pFile, sizeof(uint32_t)*2+sizeof(uint64_t)*code, SEEK_SET);
	fread(&offset_s, sizeof(uint64_t), 1, pFile);
	offset_s += sizeof(uint32_t)*2*8 + sizeof(uint64_t)*(codeNum+1)*8;
	fseek(pFile, sizeof(uint32_t)*2+sizeof(uint64_t)*(code+1), SEEK_SET);
	fread(&offset_e, sizeof(uint64_t), 1, pFile);
	offset_e += sizeof(uint32_t)*2*8 + sizeof(uint64_t)*(codeNum+1)*8;
	//printf("codenum:%d  codelen:%d  offset:[%d, %d]\n", codeNum, codeLen, offset_s, offset_e);
	return GetSetsFromFile(pFile, offset_s, offset_e, codeLen);
}

//setList : [left1, right1], [left2, right2], ...
void Retrieval::GetItemsBySets(uint32_t ** items, const uint32_t&cnt, std::list<Interval> *setList)
{
	char filePath[] = "./out/compressedLogs/logs-00";
	uint32_t *buffer = new uint32_t [2048000];
	uint32_t *codeArr = new uint32_t [2048000];
	uint32_t *prefixArr = new uint32_t [2048000];
	for(uint32_t c=0; c<CONFIGURATION.getAttributesNum(); c++) {
        //if(c == 13 || c == 14) continue;
		filePath[strlen(filePath)-2] = '0' + c/10;
		filePath[strlen(filePath)-1] = '0' + c%10;
		FILE *pFile = fopen(filePath, "rb");
		uint32_t cnt = 0;
		uint32_t readNum = 0;
		while((readNum=fread(buffer+cnt, 4, 1024000, pFile)) > 0) {
			cnt += readNum;
		}
		uint32_t size = buffer[0];
		uint32_t codeLen = buffer[1];
		uint32_t lineLen = buffer[2];
		uint32_t idx_code = 0;
		uint32_t idx_prefix = 0;
		uint32_t offset = 0;
		uint32_t idx = 3;
		for(uint32_t i=0; i<size/2; i++) {
			if(get_from_bit(buffer+idx, offset, codeArr[idx_code++], codeLen)) idx++;
			if(get_from_bit(buffer+idx, offset, prefixArr[idx_prefix++], lineLen)) idx++;
		}
		idx = 0;
		uint32_t lbound = 0;
		uint32_t rbound = 0;
		uint32_t linecnt = 0;
		for(std::list<Interval>::iterator it=setList->begin(); it!=setList->end(); it++) {
			uint32_t left = it->L;
			uint32_t right = it->R;
			idx = lbound = std::lower_bound(prefixArr+idx, prefixArr+idx_prefix, left) - prefixArr;
			idx = rbound = std::lower_bound(prefixArr+idx, prefixArr+idx_prefix, right) - prefixArr;
			//printf("lbound = %d  rbound = %d\n", lbound, rbound);
			//printf("code: ");for(uint32_t x=lbound; x<=rbound; x++) printf("%d ", codeArr[x]); puts("");
			if(lbound == rbound) {
				for(uint32_t i=left; i<=right; i++) items[linecnt++][c] = codeArr[lbound];
			} else {
				for(uint32_t i=left; i<=prefixArr[lbound]; i++) items[linecnt++][c] = codeArr[lbound];
				for(lbound=lbound+1; lbound < rbound; lbound++) {
					for(uint32_t j=prefixArr[lbound-1]+1; j<=prefixArr[lbound]; j++)
						items[linecnt++][c] = codeArr[lbound];
				}
				for(uint32_t i=prefixArr[rbound-1]+1; i<=right; i++)
					items[linecnt++][c] = codeArr[lbound];
			}
		}
		//if(linecnt != cnt) printf("linecnt=%d  line=%d\n", linecnt, line);
		//assert(linecnt == cnt);
		fclose(pFile);
	}
	delete[]buffer;
	delete[]codeArr;
	delete[]prefixArr;
}

uint32_t ** Retrieval::Search(const uint32_t &cID, const char *value, uint32_t &cnt) {
	DataType dt = CONFIGURATION.getAttributeVec()[cID].type;
	uint32_t code = 0;
	uint32_t key = -1;
	if(dt == TIME) {
		key = convertTimeStrToInt32(value);
		code = GetCodeByInt32Value(cID, key);
	} else if(dt == IPv4) {
		key = convertIPv4ToInt32(value);
		code = GetCodeByInt32Value(cID, key);
	} else if(dt == PORT) {
		key = convertPortToInt16(value);
		code = GetCodeByInt16Value(cID, key);
	} else {
		code = GetCodeByStrValue(cID, value);
	}
	std::list<Interval> *setList = GetSetsByCode(cID, code);
	cnt = 0;
	for(std::list<Interval>::iterator it=setList->begin(); it!=setList->end(); it++) {
		//printf("[%u, %u]\n", it->L, it->R);
        cnt += it->R - it->L + 1;
	}
	uint32_t **rt = new uint32_t*[cnt];
	uint32_t attributesNum = CONFIGURATION.getAttributesNum();
	for(uint32_t i=0; i<cnt; i++) {
		rt[i] = new uint32_t [attributesNum];
	}
	GetItemsBySets(rt, cnt, setList);
	delete setList;
	return rt;
}


//=================== Display and Check Results ===============================
void GetStrOrIntByCode(const uint32_t &code, const uint32_t &cID, uint32_t &value, char* str) {
	char input_path[] = "./out/hash/hash-00";
	input_path[strlen(input_path)-2] = '0' + cID/10;
	input_path[strlen(input_path)-1] = '0' + cID%10;
	if(CONFIGURATION.getAttributeVec()[cID].type == TIME ||
			CONFIGURATION.getAttributeVec()[cID].type == IPv4) {
		IOBuffer<uint32_t> *buffer_in = new IOBuffer<uint32_t>(input_path, "rb", 160000);
		for(uint32_t i=0; i<=code; i++) buffer_in->getNextInt(value);
		delete buffer_in;
		return;
	} else if(CONFIGURATION.getAttributeVec()[cID].type == PORT) {
		IOBuffer<uint16_t> *buffer_in = new IOBuffer<uint16_t>(input_path, "rb", 38000);
		for(uint32_t i=0; i<=code; i++) buffer_in->getNextInt(value);
		delete buffer_in;
		return;
	}
	else {
		IOBuffer<char> *buffer_in = new IOBuffer<char>(input_path, "r", 1024000);
		for(uint32_t i=0; i<=code; i++) buffer_in->getNextLineCopy(str);
		//printf("get, code=%u  str=%s\n", code, str);
		delete buffer_in;
		return;
	}
}

void Retrieval::DisplayResults(uint32_t **items, const uint32_t &cnt) {
	printf("count : %u lines\n", cnt);
	uint32_t attributesNum = CONFIGURATION.getAttributesNum();
	char str[1000];
	uint32_t len;
	uint32_t value;
	for(uint32_t i=0; i<cnt; i++) {
		for(uint32_t j=0; j<attributesNum; j++) {
			value = -1;
			memset(str, 0, sizeof(str));
			GetStrOrIntByCode(items[i][j], j, value, str);
			if(CONFIGURATION.getAttributeVec()[j].type == TIME) {
				DisplayTime(value);
			} else if(CONFIGURATION.getAttributeVec()[j].type == IPv4) {
				DisplayIPv4(value);
			} else if(CONFIGURATION.getAttributeVec()[j].type == PORT) {
				DisplayPort(value);
			} else {
				printf("%s", str);
			}
			if(j != attributesNum-1) putchar(',');
			else puts("");
		}
	}
}
//=============================================================================
#endif /* SRC_RETRIEVAL_H_ */
