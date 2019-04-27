//
//	Logs.h
//	LogCompression
//	
//	Created by Chongbo Wei on 2019/4/12
//	Copyright © 2019 Chongbo Wei. All rights reserved.

#ifndef LOGS_H_
#define LOGS_H_

#include "base.h"
#include "config_parser.h"
#include "HashTable.h"
#include "IOBuffer.h"

class OneAttributeList {
private:
	char **List;
	uint32_t listLen;
	DataType type;

public:
	OneAttributeList(char ***lines, uint32_t lineNums, uint32_t volumnID) {
		this->List = new char*[lineNums];
		this->listLen = lineNums;
		this->type = CONFIGURATION.getAttributeVec()[volumnID].type;
		for(uint32_t i=0; i<lineNums; i++) {
			this->List[i] = lines[i][volumnID];
		}
	}

	~OneAttributeList() {
		delete []List;
	}

	DataType getDataType() {
		return this->type;
	}

	uint32_t getListLen() {
		return this->listLen;
	}

	char** getList() {
		return this->List;
	}

};

//a part of all logs, because logs are too large to load in memory
class LogPiece {
public:
	uint32_t pieceID;		//which part of origin logs
	char ***lines;			//malloc in its constructor
	uint32_t preMallocLen;
	uint32_t lineNums;		//
	uint32_t memoryLimit;	//Mb


public:
	LogPiece(uint32_t pieceID, uint32_t lineLimit, uint32_t memoryLimit) {
		//if(DEBUG) PRINT_LOG("in LogPiece()\n");
		this->pieceID = pieceID;
		this->preMallocLen = std::min(lineLimit, (uint32_t)((uint64_t)memoryLimit*1024*1024/4/CONFIGURATION.getAttributesNum()));
		this->lineNums = 0;
		this->memoryLimit = memoryLimit;
		this->lines = new char** [this->preMallocLen];
		for(uint32_t i=0; i<this->preMallocLen; i++) {
			this->lines[i] = new char* [CONFIGURATION.getAttributesNum()];
		}
		//if(DEBUG) PRINT_LOG("in LogPiece, id:%d, lineNum:%d memlimit:%d\n", pieceID, lineNums, memoryLimit);
	}

	~LogPiece() {
		for(uint32_t i=0; i<this->preMallocLen; i++) {
			delete[] lines[i];
		}
		delete []lines;
	}

};

#endif /* LOGS_H_ */
