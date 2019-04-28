//
//	base.h
//	LogCompression
//	
//	Created by Chongbo Wei on 2019/4/12
//	Copyright © 2019 Chongbo Wei. All rights reserved.

#ifndef BASE_H_INCLUDED
#define BASE_H_INCLUDED

#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <list>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <yaml-cpp/yaml.h>
#include <ctime>

#define PRINT_LOG printf("[%s:%d] ",__FILE__, __LINE__); printf
#define DEBUG true

//================== Data Structure ====================
enum DataType {
	STRING, BIT, TEXT, BOOL, TIME, IPv4, PORT, UNKNOWN
};

const char * const dataTypeArr[] = {
	"string", "bit", "text", "bool", "time", "ipv4", "port", "unknown"};


struct Attribute {
	std::string name;
	DataType type;
};

struct Attributes {
	uint32_t num;
	char separator;
	std::vector<Attribute> attributeVec;
};

template <typename T1, typename T2>
struct Pair {
	T1 key;
	T2 value;
	Pair(T1 x, T2 y):key(x), value(y) {}
};

struct Interval {
    uint32_t L;
    uint32_t R;
    Interval(uint32_t L, uint32_t R):L(L), R(R) {}
};

extern	uint32_t cc2num['9'+1]['9'+1];
extern	uint32_t c2num['9'+1];

//==================== function ======================

void initcc2num();
DataType getDateTypeByStr(const char* str);
void judge();

//print x to str's uint32_t array
inline bool print_to_bit(uint32_t *str, uint32_t &offset, uint32_t x, const uint32_t &bs) {
	str[0] |= x<<(32-bs)>>offset;
	if(bs + offset >= 32) {
		str++;
		offset = bs+offset-32;
		if(offset) str[0] |= x<<(32-offset);
		return true;
	}
	offset += bs;
	return false;
}

inline bool get_from_bit(uint32_t *str, uint32_t &offset, uint32_t &x, const uint32_t &bs) {
	x = 0;
	x |= str[0]<<offset>>(32-bs);
	if(bs + offset >= 32) {
		str++;
		offset = bs+offset-32;
		if(offset) x |= str[0]>>(32-offset);
		return true;
	}
	offset += bs;
	return false;
}

inline uint32_t convertTimeStrToInt32(const char *str) {
	uint32_t rt = 0;
	rt |= (cc2num[str[0]][str[1]] * 100 + cc2num[str[2]][str[3]]) - 2018 << 26;
	rt |= cc2num[str[5]][str[6]] << 22;
	rt |= cc2num[str[8]][str[9]] << 17;
	rt |= cc2num[str[11]][str[12]] << 12;
	rt |= cc2num[str[14]][str[15]] << 6;
	rt |= cc2num[str[17]][str[18]];
	return rt;
	/*
	uint32_t Y = (str[0]-'0')*1000 + (str[1]-'0')*100 + (str[2]-'0')*10 + str[3]-'0';
	uint32_t M = (str[5]-'0')*10 + str[6]-'0';
	uint32_t D = (str[8]-'0')*10 + str[9]-'0';
	uint32_t h = (str[11]-'0')*10 + str[12]-'0';
	uint32_t m = (str[14]-'0')*10 + str[15]-'0';
	uint32_t s = (str[17]-'0')*10 + str[18]-'0';
	return (Y-2018)<<26 | M<<22 | D<<17 | h<<12 | m<<6 | s ;
	*/
}

inline uint32_t convertIPv4ToInt32(const char *str) {
	uint32_t idx = 0;
	uint32_t tp = 0;
	uint32_t rt = 0;
	while(str[idx] != '.') tp = tp *10 + c2num[str[idx++]]; idx ++;
	rt |= tp<<24;
	tp = 0;
	while(str[idx] != '.') tp = tp *10 + c2num[str[idx++]]; idx ++;
	rt |= tp<<16;
	tp = 0;
	while(str[idx] != '.') tp = tp *10 + c2num[str[idx++]]; idx ++;
	rt |= tp<<8;
	tp = 0;
	while(str[idx]>='0' && str[idx]<='9') tp = tp *10 + c2num[str[idx++]];
	return rt | tp;
}

inline uint16_t convertPortToInt16(const char *str) {
	uint32_t idx = 0;
	uint32_t rt = 0;
	while(str[idx]>='0' && str[idx]<='9') rt = rt *10 + c2num[str[idx++]];
	return rt;
}

inline void DisplayTime(const uint32_t &value) {
	uint32_t Y = (value>>26 & ((1<<6)-1)) + 2018; //value >>= 6;
	uint32_t M = value>>22 & ((1<<4)-1);
	uint32_t D = value>>17 & ((1<<5)-1);
	uint32_t h = value>>12 & ((1<<5)-1);
	uint32_t m = value>>6 & ((1<<6)-1);
	uint32_t s = value & ((1<<6)-1);
	printf("%4d/%02d/%02d %02d:%02d:%02d", Y, M, D, h, m, s);
}

inline void DisplayIPv4(const uint32_t &value) {
	uint32_t mask = (1<<8)-1;
	uint32_t a = value>>24 & mask;
	uint32_t b = value>>16 & mask;
	uint32_t c = value>>8 & mask;
	uint32_t d = value & mask;
	printf("%d.%d.%d.%d", a, b, c, d);
}

inline void DisplayPort(const uint32_t &value) {
	printf("%u", value);
}

#endif // BASE_H_INCLUDED
