//
//	base.cpp
//	LogCompression
//	
//	Created by Chongbo Wei on 2019/4/12
//	Copyright © 2019 Chongbo Wei. All rights reserved.

#include "base.h"

DataType getDateTypeByStr(const char* str) {
	for(int i=0; i<9; i++) {
		if(strcmp(str, dataTypeArr[i]) == 0)
			return static_cast<DataType>(i);
	}
	return UNKNOWN;
}

uint32_t cc2num['9'+1]['9'+1];
uint32_t c2num['9'+1];

void initcc2num() {
	for(int i='0'; i<='9'; i++) {
		c2num[i] = i - '0';
		for(int j='0'; j<='9'; j++) {
			cc2num[i][j] = (i-'0')*10 + (j-'0');
		}
	}
}

void judge() {
    FILE* f1 = fopen("results.txt","r");
    FILE* f2 = fopen("results1.txt","r");
    if(f1 == NULL || f2 == NULL) { puts("FILE open failed"); }
    int r = 1, c = 1;
    char c1, c2;
    while(fscanf(f1,"%c",&c1) == 1) {
        fscanf(f2,"%c",&c2);
        if(c1 == '\n') {
            r++, c=1;
        }
        else c++;
        if(c1 != c2) {
            puts("Wrong Answer");
            printf("R = %d  C = %d\n",r,c);
            puts(" --- ");
            printf("Expected %c but got an %c\n",c1,c2);
        }
    }
    if(fscanf(f2,"%c",&c2) == 1) puts("Output Limited");
    else puts("Acceptted");
}

/*
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
	rt |= (((str[0]-'0')*1000 + (str[1]-'0')*100 + (str[2]-'0')*10 + str[3]-'0')-2018) << 26;
	rt |= ((str[5]-'0')*10 + str[6]-'0') << 22;
	rt |= ((str[8]-'0')*10 + str[9]-'0') << 17;
	rt |= ((str[11]-'0')*10 + str[12]-'0') << 12;
	rt |= ((str[14]-'0')*10 + str[15]-'0') << 6;
	rt |= (str[17]-'0')*10 + str[18]-'0';
	return rt;
}

inline uint32_t convertIPv4ToInt32(const char *str) {
	uint32_t idx = 0;
	uint32_t tp = 0;
	uint32_t rt = 0;
	while(str[idx] != '.') tp = tp *10 + str[idx++]-'0'; idx ++;
	rt |= tp<<24;
	tp = 0;
	while(str[idx] != '.') tp = tp *10 + str[idx++]-'0'; idx ++;
	rt |= tp<<16;
	tp = 0;
	while(str[idx] != '.') tp = tp *10 + str[idx++]-'0'; idx ++;
	rt |= tp<<8;
	tp = 0;
	while(str[idx]>='0' && str[idx]<='9') tp = tp *10 + str[idx++]-'0';
	return rt;
}

inline uint16_t convertPortToInt16(const char *str) {
	uint32_t idx = 0;
	uint32_t rt = 0;
	while(str[idx]>='0' && str[idx]<='9') rt = rt *10 + str[idx++]-'0';
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
	uint32_t a = value>>24 & ((1<<8)-1);
	uint32_t b = value>>16 & ((1<<8)-1);
	uint32_t c = value>>8 & ((1<<8)-1);
	uint32_t d = value & ((1<<8)-1);
	printf("%d.%d.%d.%d", a, b, c, d);
}
inline void DisplayPort(const uint32_t &value) {
	printf("%u", value);
}
*/
