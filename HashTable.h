//
//	HashTable.h
//	LogCompression
//	
//	Created by Chongbo Wei on 2019/4/12
//	Copyright © 2019 Chongbo Wei. All rights reserved.

#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include "base.h"
#include "IOBuffer.h"

class HashTable {
public:
    DataType type;
	uint32_t tableSize;
public:
	HashTable(): tableSize(0){};
	virtual ~HashTable() = default;
	virtual uint32_t insert(const char*str) = 0;
	//virtual uint32_t insert(const uint32_t &key){};
	virtual void DumpToFile(const char *output_path) = 0;
	virtual uint64_t GetDiskConsumption() = 0;
};

class HashTable_string : public  HashTable {
public:
	std::vector<const char*> id2val;

private:
	std::list<Pair<const char*, uint32_t> > *table;	//table[]

public:
	HashTable_string(DataType type, uint32_t  table_size) {
        this->type = type;
		this->tableSize = table_size;
		this->id2val.clear();
		this->table = new std::list<Pair<const char*, uint32_t> >[table_size];
	}

	~HashTable_string() {
		for(uint32_t i=0; i<this->id2val.size(); i++) {
			delete id2val[i];
		}
		delete []table;
	}

	uint32_t hash(const char *str) {
		uint32_t h = 0;
		uint32_t g;
		uint32_t idx = 0;
	    while (*str)
	    {
	        h = (h << 4) + *str++;
	        g = h & 0xF0000000;
	        if (g)
	        {
	            h ^= (g >> 24);
	        }
	        h &= ~g;
	    }
	    return h % this->tableSize;
	}

	uint32_t find(const char* str, const uint32_t &hashValue) {
		std::list<Pair<const char*, uint32_t> > &bucket = this->table[hashValue];
		for(std::list<Pair<const char*, uint32_t> >::iterator it=bucket.begin(); it!=bucket.end(); it++) {
			if(strcmp(it->key, str) == 0) {
				return it->value;
			}
		}
		return -1;
	}

	virtual uint32_t insert(const char *str) {
		uint32_t hashValue = hash(str);
		uint32_t rt_id;
		if((rt_id = find(str, hashValue)) != -1) {
			return rt_id;
		}
		/////////// deep copy  and insert //////////
		char *copy = new char[strlen(str)+1];
		strcpy(copy, str);
		rt_id = this->id2val.size();
		this->id2val.push_back(copy);
		this->table[hashValue].push_back(Pair<const char*, uint32_t>(copy, rt_id));
		return rt_id;
	}

	virtual void DumpToFile(const char *output_path) {
		uint32_t bufferLen = std::min((uint64_t)10240000, (uint64_t)CONFIGURATION.getMemoryLimit()*1024*1024/5);
        IOBuffer<char> *buffer_out = new IOBuffer<char>(output_path, "a+", bufferLen);
        buffer_out->writeStrArray(this->id2val);
        delete buffer_out;
		printf("writed %s\n", output_path);
	}

	virtual uint64_t GetDiskConsumption() {
		uint32_t ans = 0;
		for(uint32_t i=0; i<this->id2val.size(); i++) {
			ans += strlen(id2val[i]+1) * sizeof(char);
		}
		return ans;
	}

};

class HashTable_int : public  HashTable {
public:
	std::vector<uint32_t> id2val;

private:
	std::list<Pair<uint32_t, uint32_t> > *table;	//table[]

public:
	HashTable_int(const DataType &type, const uint32_t  &table_size) {
        this->type = type;
		this->tableSize = table_size;
		this->id2val.clear();
		this->table = new std::list<Pair<uint32_t, uint32_t> >[table_size];
	}

	~HashTable_int() {
		delete []table;
	}

	uint32_t find(const uint32_t &key, const uint32_t &hashValue) {
		std::list<Pair<uint32_t, uint32_t> > &bucket = this->table[hashValue];
		for(std::list<Pair<uint32_t, uint32_t> >::iterator it=bucket.begin(); it!=bucket.end(); it++) {
			if(it->key == key) {
				return it->value;
			}
		}
		return -1;
	}

	virtual uint32_t insert(const char*str) {
		uint32_t key = 0;
		if(this->type == TIME) {
			key = convertTimeStrToInt32(str);
		} else if(this->type == IPv4) {
			key = convertIPv4ToInt32(str);
		} else if(this->type == PORT) {
			key = convertPortToInt16(str);
		} else {
			puts("unknow type");
			exit(-1);
		}
		uint32_t hashValue = key % this->tableSize;
		uint32_t rt_id;
		if((rt_id = find(key, hashValue)) != -1) {
			return rt_id;
		}
		rt_id = this->id2val.size();
		this->id2val.push_back(key);
		this->table[hashValue].push_back(Pair<uint32_t, uint32_t>(key, rt_id));
		return rt_id;
	}

	virtual void DumpToFile(const char *output_path) {
		uint32_t bufferLen = std::min((uint64_t)10240000, (uint64_t)CONFIGURATION.getMemoryLimit()*1024*1024/20);
		if(this->type == PORT) {
			IOBuffer<uint16_t> *buffer_out = new IOBuffer<uint16_t>(output_path, "ab+", bufferLen<<1);
			buffer_out->writeIntArray(this->id2val);
			delete buffer_out;
		}
		else {
			IOBuffer<uint32_t> *buffer_out = new IOBuffer<uint32_t>(output_path, "ab+", bufferLen);
			buffer_out->writeIntArray(this->id2val);
			delete buffer_out;
		}
		printf("writed %s\n", output_path);
	}

	virtual uint64_t GetDiskConsumption() {
		uint32_t ans = 0;
		return (uint64_t)this->id2val.size() * 4;
		return ans;
	}

};


#endif /* HASHTABLE_H_ */
