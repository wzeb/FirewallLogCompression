//
//	IOBuffer.h
//	LogCompression
//	
//	Created by Chongbo Wei on 2019/4/12
//	Copyright © 2019 Chongbo Wei. All rights reserved.

#ifndef IOBUFFER_H_
#define IOBUFFER_H_
#include "base.h"
#include "config_parser.h"

template<typename T>
class IOBuffer {
private:
	FILE *pfile;
	//uint64_t file_offset;	//file offset, seek use
	T *buffer;
	uint32_t idx_start;
	uint32_t idx_end;
	uint32_t bufferSize;

public:
	IOBuffer(const char *filepath, const char *style, uint32_t buffersize) {
		if((this->pfile=fopen(filepath, style)) == NULL) {
			std::cout << "error: open '" << filepath << "' failed." << std::endl;
		}
		//this->file_offset = 0;
		buffersize = buffersize / (4096/sizeof(T)) * (4096/sizeof(T));
		assert(buffersize > 0);
		this->bufferSize = buffersize;
		this->buffer = new T[buffersize+1]();
		this->idx_start = this->idx_end = 0;
	}

	/*
	IOBuffer(const FILE* pFile, uint32_t buffersize) {
		this->pfile = pFile;
		//this->file_offset = 0;
		this->bufferSize = buffersize;
		this->buffer = new T[buffersize+1]();
		this->idx_start = this->idx_end = 0;
	}
	*/

	~IOBuffer() {
		delete []this->buffer;
		fclose(this->pfile);
	}

	void changeFile(const char *filepath, const char *style) {
		fclose(this->pfile);
		if((this->pfile=fopen(filepath, style)) == NULL) {
			std::cout << "error: open '" << filepath << "' failed." << std::endl;
		}
		//this->file_offset = 0;
		this->idx_start = this->idx_end = 0;
	}

	bool getNextItem(char **item)
	{
		// check if have whole item
		uint32_t idx = this->idx_start;
		while(idx < this->idx_end && buffer[idx]!='\n') idx++;
		if(idx == this->idx_end) {
			//fseek(this->pfile, this->file_offset, SEEK_SET);
			memcpy(buffer, buffer+this->idx_start, (this->idx_end-this->idx_start)*sizeof(buffer[0]));
			this->idx_end = this->idx_end - this->idx_start;
			this->idx_start = 0;
			uint32_t freeSpace = this->bufferSize-this->idx_end;
			freeSpace = freeSpace / 4096 * 4096;
			assert(freeSpace > 0);
			uint32_t read_num = fread(buffer+this->idx_end, sizeof(buffer[0]), freeSpace, this->pfile);
			if(!read_num) return false;
			//this->file_offset += read_num;
			this->idx_end += read_num;
		}
		////////////////////////////////////////////////////////////////
		uint32_t attributes_num = CONFIGURATION.getAttributes().num;
		char separator = CONFIGURATION.getAttributes().separator;
		for(uint32_t i=0; i<attributes_num-1; i++) {
			while(buffer[this->idx_start] == ' ') this->idx_start++;
			item[i] = &buffer[this->idx_start];
			while(buffer[this->idx_start] != separator) this->idx_start++;
			buffer[this->idx_start++] = 0;
		}
		//read last util to end
		while(buffer[this->idx_start] == ' ') this->idx_start++;
		item[attributes_num-1] = &buffer[this->idx_start];
		while(buffer[this->idx_start] != '\n')  this->idx_start++;
		buffer[this->idx_start++] = 0;
		///////////////////////////////////////////////////////////////
		return true;
	}

	bool getNextLine(char *&str) {
		// judge if have whole str
		uint32_t idx = this->idx_start;
		while(idx < this->idx_end && buffer[idx]!='\n') idx++;
		if(idx == this->idx_end) {
			//fseek(this->pfile, this->file_offset, SEEK_SET);
			memcpy(buffer, buffer+this->idx_start, (this->idx_end-this->idx_start)*sizeof(buffer[0]));
			this->idx_end = this->idx_end - this->idx_start;
			this->idx_start = 0;
			uint32_t freeSpace = this->bufferSize-this->idx_end;
			freeSpace = freeSpace / 4096 * 4096;
			assert(freeSpace > 0);
			uint32_t read_num = fread(buffer+this->idx_end, sizeof(buffer[0]), freeSpace, this->pfile);
			if(!read_num) return false;
			//this->file_offset += read_num;
			this->idx_end += read_num;
		}
		////////////////////////////////////////////////////////////////
		//for(int x=0; x<10; x++) printf("%d ", buffer[x]);puts("");
		while(buffer[this->idx_start] == ' ') (this->idx_start)++;
		str = &buffer[this->idx_start];
		while(buffer[this->idx_start] != '\n')  (this->idx_start)++;
		buffer[this->idx_start++] = 0;
		return true;
	}

	bool getNextLineCopy(char *str) {
		// judge if have whole str
		uint32_t idx = this->idx_start;
		while(idx < this->idx_end && buffer[idx]!='\n') idx++;
		if(idx == this->idx_end) {
			//fseek(this->pfile, this->file_offset, SEEK_SET);
			memcpy(buffer, buffer+this->idx_start, (this->idx_end-this->idx_start)*sizeof(buffer[0]));
			this->idx_end = this->idx_end - this->idx_start;
			this->idx_start = 0;
			uint32_t freeSpace = this->bufferSize-this->idx_end;
			freeSpace = freeSpace / 4096 * 4096;
			assert(freeSpace > 0);
			uint32_t read_num = fread(buffer+this->idx_end, sizeof(buffer[0]), freeSpace, this->pfile);
			if(!read_num) return false;
			//this->file_offset += read_num;
			this->idx_end += read_num;
		}
		////////////////////////////////////////////////////////////////
		//for(int x=0; x<10; x++) printf("%d ", buffer[x]);puts("");
		while(buffer[this->idx_start] == ' ') (this->idx_start)++;
		char *start = &buffer[this->idx_start];
		while(buffer[this->idx_start] != '\n')  (this->idx_start)++;
		buffer[this->idx_start++] = 0;
		strcpy(str, start);
		return true;
	}

	bool getNextInt(uint32_t &itg) {
		if(this->idx_start == this->idx_end) {
			this->idx_start = 0;
			this->idx_end = fread(buffer, sizeof(T), this->bufferSize, this->pfile);
			if(!this->idx_end) return false;
		}
		itg = (uint32_t)this->buffer[this->idx_start++];
		return true;
	}

	void writeIntArray(std::vector<uint32_t> &vec) {
		this->idx_end = 0;
		size_t id = 0;
		while(id < vec.size()) {
			if(idx_end == this->bufferSize) {
				fwrite(this->buffer, sizeof(T), this->bufferSize, this->pfile);
				fflush(this->pfile);
				this->idx_end = 0;
			}
			this->buffer[this->idx_end++] = (T)vec[id++];
		}
		fwrite(this->buffer, sizeof(T), idx_end, this->pfile);
		fflush(this->pfile);
	}

	void writeStrArray(std::vector<const char*> &vec) {
		assert(sizeof(T) == 1);
		this->idx_end = 0;
		size_t id = 0;
		uint32_t freeSpace = this->bufferSize;
		uint32_t strLen = 0;
		while(id < vec.size()) {
			strLen = strlen(vec[id])+1;
			if(freeSpace < strLen) {
				this->buffer[this->idx_end] = 0;
				fputs(this->buffer, this->pfile);
				fflush(this->pfile);
				this->idx_end = 0;
				freeSpace = this->bufferSize;
				continue;
			}
			memcpy(this->buffer+this->idx_end, vec[id], strLen);
			this->idx_end += strLen;
			this->buffer[this->idx_end-1] = '\n';
			freeSpace -= strLen;
			id++;
		}
		this->buffer[this->idx_end] = 0;
		fputs(this->buffer, this->pfile);
		fflush(this->pfile);
	}

	void writeCompressedLogs(std::list<uint32_t> &logs, uint32_t codeLen, uint32_t lineLen) {
		assert(sizeof(T) == 4);
		uint32_t size = logs.size();
		fwrite(&size, sizeof(size), 1, this->pfile);
		fwrite(&codeLen, sizeof(codeLen), 1, this->pfile);
		fwrite(&lineLen, sizeof(lineLen), 1, this->pfile);
		fflush(this->pfile);
		this->idx_end = 0;
		std::list<uint32_t>::iterator it = logs.begin();
		uint64_t freeSpace = (uint64_t)this->bufferSize*32;
		uint32_t offset = 0;
		while(it != logs.end()) {
			if(print_to_bit(this->buffer+this->idx_end, offset, *it, codeLen)) {
				this->idx_end++;
			}
			freeSpace -= codeLen;
			if(freeSpace < 0) {
				fwrite(this->buffer, sizeof(this->buffer[0]), this->bufferSize, this->pfile);
				fflush(this->pfile);
				offset = 0;
				this->idx_end = 0;
				if(print_to_bit(this->buffer, offset, *it, -freeSpace)) {
					this->idx_end ++;
				}
				freeSpace += (uint64_t)this->bufferSize*32;
			}
			std::swap(codeLen, lineLen);
			it++;
		}
		fwrite(this->buffer, sizeof(this->buffer[0]), this->idx_end+1, this->pfile);
		fflush(this->pfile);
	}

	//lineLen : 行号编码长度
	void DumpIndexsSet(std::vector<std::list<uint32_t> > &indexs, uint32_t lineLen, uint32_t codeNum) {
		assert(sizeof(T) == 4);
		fwrite(&codeNum, sizeof(uint32_t), 1, this->pfile);
		fwrite(&lineLen, sizeof(uint32_t), 1, this->pfile);
		fflush(this->pfile);
		// write offsets of every sets
		uint64_t Offset = 0;
		for(uint32_t i=0; i<codeNum; i++) {
			fwrite(&Offset, sizeof(uint64_t), 1, this->pfile);
			Offset += (uint64_t)indexs[i].size()*lineLen;
		}
		fwrite(&Offset, sizeof(uint64_t), 1, this->pfile);
		fflush(this->pfile);
		// write every set
		uint64_t freeSpace = (uint64_t)this->bufferSize*32;
		uint32_t offset = 0;
		this->idx_end = 0;
		for(uint32_t i=0; i<codeNum; i++) {
			std::list<uint32_t>& lst = indexs[i];
			std::list<uint32_t>::iterator it = lst.begin();
			while(it != lst.end()) {
				if(print_to_bit(this->buffer+this->idx_end, offset, *it, lineLen)) {
						this->idx_end++;
				}
				freeSpace -= lineLen;
				if(freeSpace < 0) {
					fwrite(this->buffer, sizeof(this->buffer[0]), this->bufferSize, this->pfile);
					fflush(this->pfile);
					offset = 0;
					this->idx_end = 0;
					if(print_to_bit(this->buffer, offset, *it, -freeSpace)) {
						this->idx_end ++;
					}
					freeSpace += (uint64_t)this->bufferSize*32;
				}
				it++;
			}
		}
		fwrite(this->buffer, sizeof(this->buffer[0]), this->idx_end+1, this->pfile);
		fflush(this->pfile);
	}
};

#endif /* IOBUFFER_H_ */
