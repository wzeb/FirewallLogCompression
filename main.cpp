//
//	main.h
//	LogCompression
//	
//	Created by Chongbo Wei on 2019/4/12
//	Copyright © 2019 Chongbo Wei. All rights reserved.


#include "config_parser.h"
#include "HashTable.h"
#include "Logs.h"
#include "IOBuffer.h"
#include "Compressor.h"
#include "CompressedLogs.h"
#include "Indexs.h"
#include "Retrieval.h"
#include "RudeMethod.h"

std::list<uint32_t> logs;
std::vector<std::list<uint32_t> > indexs;

void constructFileName(char *output_path, const char*type, uint32_t cID) {
	if(strcmp(type, "HashTable") == 0) {
		strcpy(output_path, "./out/hash/hash-00");
	}
	else if(strcmp(type, "CompressedLog") == 0) {
		strcpy(output_path, "./out/compressedLogs/logs-00");
	}
	else
		strcpy(output_path, "./out/indexs/index-00");
	output_path[strlen(output_path)-2] = '0' + cID/10;
	output_path[strlen(output_path)-1] = '0' + cID%10;
}

uint64_t getDiskConsumption(HashTable **hts, CompressedLogs **cls, Indexs **indexs) {
	uint32_t attributesNum = CONFIGURATION.getAttributesNum();
	uint64_t htsConsum = 0, clsConsum = 0, indexsConsum = 0;
	for(uint32_t i=0; i<attributesNum; i++) {
		htsConsum += hts[i]->GetDiskConsumption();
		clsConsum += cls[i]->GetDiskConsumption();
		indexsConsum += indexs[i]->GetDiskConsumption();
	}
	printf("HashTable:%f  CompressedLog:%f  indexs:%f\n",
			htsConsum/1024.0/1024, clsConsum/1024.0/1024, indexsConsum/1024.0/1024);
	return htsConsum + clsConsum + indexsConsum;
}

void Compress() {
	uint32_t attributesNum = CONFIGURATION.getAttributesNum();
	uint32_t bufferLen = std::min((uint64_t)102400000, (uint64_t)CONFIGURATION.getMemoryLimit()*1024*1024/5);
	IOBuffer<char> *buffer_in = new IOBuffer<char>(CONFIGURATION.getFilePath().c_str(), "r", bufferLen);
	char **item = new char*[attributesNum];
	HashTable **hts = new HashTable* [attributesNum];
	CompressedLogs **cls = new CompressedLogs* [attributesNum];
	Indexs **indexs = new Indexs* [attributesNum];
	for(uint32_t i=0; i<attributesNum; i++) {
		if(CONFIGURATION.getAttributeVec()[i].type == IPv4 ||
				CONFIGURATION.getAttributeVec()[i].type == TIME ||
				CONFIGURATION.getAttributeVec()[i].type == PORT) {
			hts[i] = new HashTable_int(CONFIGURATION.getAttributeVec()[i].type, 2000003);
		} else {
			hts[i] = new HashTable_string(CONFIGURATION.getAttributeVec()[i].type, 2000003);
		}
		cls[i] = new CompressedLogs();
		indexs[i] = new Indexs();
	}
	uint32_t count = 0;
	uint32_t hashCode = -1;
	uint32_t idx = 0;
	while (buffer_in->getNextItem(item)) {
		for(idx=0; idx<attributesNum; idx++) {
			hashCode = hts[idx]->insert(item[idx]);
			cls[idx]->AddCompressedLog(hashCode);
			indexs[idx]->AddIndex(hashCode, count);
		}
		count ++;
	}
	char *output_path = new char[50];
	for(uint32_t i=0; i<attributesNum; i++) {
		constructFileName(output_path, "HashTable", i);
		hts[i] -> DumpToFile(output_path);
		constructFileName(output_path, "CompressedLog", i);
		cls[i] -> DumpToFile(output_path);
		constructFileName(output_path, "IndexsSet", i);
		indexs[i] -> DumpToFile(output_path);
	}
	delete buffer_in;
	for(uint32_t i=0; i<CONFIGURATION.getAttributesNum(); i++) {
		delete hts[i];
		delete cls[i];
		delete indexs[i];
	}
	delete []hts;
	delete []cls;
	delete []indexs;
	delete[] item;
	//printf("Compress completed, account : %u\n", count);
}

void testMySearch() {
	uint32_t cnt = 0;
	uint32_t **items = Retrieval::Search(4, "210.75.253.243", cnt);
	Retrieval::DisplayResults(items, cnt);
}

void testRudeSearch() {
	uint32_t cnt = 0;
	std::list<char**> items = RudeMethod::Search(4, "210.75.253.243", cnt);
}

std::list<char**> originalSearch(const uint32_t &cID, const char *value, uint32_t &cnt) {
	std::list<char**> rt;
	uint32_t attributesNum = CONFIGURATION.getAttributesNum();
	uint32_t bufferLen = std::min((uint64_t)10240000, (uint64_t)CONFIGURATION.getMemoryLimit()*1024*1024/10);
	IOBuffer<char> *buffer_in = new IOBuffer<char>(CONFIGURATION.getFilePath().c_str(), "r", bufferLen);
	char **item = new char*[attributesNum];
	while (buffer_in->getNextItem(item)) {
		if(strcmp(item[cID], value) == 0) {
			rt.push_back(item);
		}
	}
	cnt = rt.size();
	delete buffer_in;
	return rt;
}

void CompareTime() {
	IOBuffer<char> *buffer_in = new IOBuffer<char>("name.list", "r", 10240000);
	char *str[1000];
	uint32_t cnt = 0;
	uint32_t retrievalNum = 0;
	while(buffer_in->getNextLine(str[retrievalNum])) retrievalNum++;
	printf("wait ...");
	clock_t start = clock();
	for(int i=0; i<retrievalNum; i++) {
		//printf("str = %s\n", str[i]);
		//clock_t bf = clock();
		uint32_t ** ans = Retrieval::Search(0, str[i], cnt);
		for(uint32_t j=0; j<cnt; j++) delete ans[j];
		delete []ans;
		//printf("cnt[%d] = %d\n", i, cnt);
		//clock_t af = clock();
		//printf("retrieval[%d] = %f\n", i, (double)(af-bf) / CLOCKS_PER_SEC);
	}
	clock_t end = clock();
	printf("\nAll retrievaled, my method cost %.2fs\nwait ...", (double)(clock()-start) / CLOCKS_PER_SEC);
	start = clock();
	for(int i=0; i<retrievalNum; i++) {
		//clock_t bf = clock();
		std::list<char**> lst = originalSearch(0, str[i], cnt);
		//clock_t af = clock();
		//printf("cnt[%d] = %d\n", i, lst.size());
		//printf("retrieval[%d] = %f\n", i, (double)(af-bf) / CLOCKS_PER_SEC);
	}
	end = clock();
	printf("\nOriginal retrieval cost %.2fs\n", (double)(end-start) / CLOCKS_PER_SEC);
	delete buffer_in;
}

void judgeMatchTV(char *str) {
    char type[100];
    char value[100];
    memset(type, 0, sizeof(type));
    memset(value, 0, sizeof(value));
    while(sscanf(type, "type=%s", type) == 1) {
        assert(sscanf(value, "value=%s", value)==1);
    }
}

void testExtraInfo()
{
    uint32_t attributesNum = CONFIGURATION.getAttributesNum();
    uint32_t bufferLen = std::min((uint64_t)102400000, (uint64_t)CONFIGURATION.getMemoryLimit()*1024*1024/5);
    IOBuffer<char> *buffer_in = new IOBuffer<char>(CONFIGURATION.getFilePath().c_str(), "r", bufferLen);
    char **item = new char*[attributesNum];
    while (buffer_in->getNextItem(item)) {
        judgeMatchTV(item[16]);
    }
    delete buffer_in;
}

void testFseek()
{
	IOBuffer<char> *buffer_in = new IOBuffer<char>("name.list", "r", 512);
	char *str[1000];
	uint32_t cnt = 0;
	uint32_t retrievalNum = 0;
	while(buffer_in->getNextLine(str[retrievalNum])) {
		puts(str[retrievalNum]);
		retrievalNum++;
	}
	delete buffer_in;
}

/*
int main()
{
	setbuf(stdout,NULL);
	freopen("results1.txt", "w", stdout);
    //Compress();
	//testRudeSearch();
	//testMySearch();
}
*/

int main(int argc, char*argv[])
{
	initcc2num();
	//freopen("log.txt", "w", stdout);
	setbuf(stdout,NULL);
	assert(argc == 2);
	assert(!strcmp(argv[1], "store") || !strcmp(argv[1], "retrieval"));
	if(!strcmp(argv[1], "store")) {
		puts("Compress ... ");
		clock_t af = clock();
		Compress();
		clock_t bf = clock();
		printf("done, cost %.2fs\n", (double)(bf-af) / CLOCKS_PER_SEC);
	} else {
		puts("Automatically retrieve the threat name that appears in 'name.list' and compare it to the original method ... ");
		CompareTime();
		puts("done.");		
	}
	//testSearch();
	//CompareTime();
	return 0;
}

/*
void Compress()
{
	HashTable **hts = new HashTable* [CONFIGURATION.getAttributesNum()];
		for(uint32_t i=0; i<CONFIGURATION.getAttributesNum(); i++) {
			hts[i] = new HashTable_string(2000003);
	}
	uint32_t bufferLen = std::min((uint64_t)102400000, (uint64_t)CONFIGURATION.getMemoryLimit()*1024*1024/5/4);
	IOBuffer<char> *buffer_in = new IOBuffer<char>(CONFIGURATION.getFilePath().c_str(), "r", bufferLen);
	uint32_t pieces_count = 0;
	while (true) {
		puts("new piece");
		LogPiece *piece = new LogPiece(pieces_count++, 102400000, (uint64_t)CONFIGURATION.getMemoryLimit()/5);
		piece->lineNums = buffer_in->getNextItems(piece->lines, piece->lineNums);
		linecount += piece->lineNums;
		if(piece->lineNums == 0) {delete piece; break;}
		for(uint32_t i=0; i<CONFIGURATION.getAttributesNum(); i++) {
			logs.clear();
			indexs.clear();
			OneAttributeList *sigleList = new OneAttributeList(piece->lines, piece->lineNums, i);
			Compressor::Compress(sigleList, hts[i], logs, indexs);
			delete sigleList;
			DumpCompressedLogs(logs, i, hts[i]->id2val.size(), piece->lineNums);
			DumpIndexsSet(indexs, i, hts[i]->id2val.size(), piece->lineNums);
		}
		delete piece;
		printf("Compressed %u lines\n", linecount);
	}
	delete buffer_in;
	for(uint32_t i=0; i<CONFIGURATION.getAttributesNum(); i++) {
		DumpHashTable(hts[i], i);
		delete hts[i];
	}
	delete []hts;
	printf("Compress completed, account : %u\n", linecount);
}
*/

/*
 * void Compress2()
{
	char output_path[] = "./out/index-00";
	HashTable **hts = new HashTable* [CONFIGURATION.getAttributesNum()];
		for(uint32_t i=0; i<CONFIGURATION.getAttributesNum(); i++) {
			hts[i] = new HashTable_string(1000003);
	}
	uint32_t bufferLen = std::min((uint64_t)102400000, (uint64_t)CONFIGURATION.getMemoryLimit()/4*1024*1024);
	printf("buffer len = %d\n", bufferLen);
	IOBuffer<char> *buffer_in = new IOBuffer<char>(CONFIGURATION.getFilePath().c_str(), "r", bufferLen);
	IOBuffer<char> *buffer_out = new IOBuffer<char>(output_path, "ab+", bufferLen);
	uint32_t pieces_count = 0;

	while (true) {
		//use 1/4 of mem
		LogPiece *piece = new LogPiece(pieces_count++, 102400000, (uint64_t)CONFIGURATION.getMemoryLimit()/4);
		piece->lineNums = buffer_in->getNextItems(piece->lines, piece->lineNums);
		linecount += piece->lineNums;
		if(piece->lineNums == 0) {delete piece; break;}
		for(uint32_t i=0; i<CONFIGURATION.getAttributesNum(); i++) {
			results.clear();
			OneAttributeList *sigleList = new OneAttributeList(piece->lines, piece->lineNums, i);
			Compressor::Compress(sigleList, hts[i], results);
			output_path[strlen(output_path)-2] = '0' + i/10;
			output_path[strlen(output_path)-1] = '0' + i%10;
			buffer_out->changeFile(output_path, "ab+");
			uint32_t IDsNum = hts[i]->id2val.size();
			printf("i = %d  IDsNum = %d\n", i, IDsNum);
			if(IDsNum < 256) buffer_out->writeResults2<uint8_t>(results);
			else if(IDsNum < 65536) buffer_out->writeResults2<uint16_t>(results);
			else buffer_out->writeResults2<uint32_t>(results);
			delete sigleList;
		}
		delete piece;
		printf("Compressed %u lines\n", linecount);
	}
	delete buffer_in;
	delete buffer_out;
	for(uint32_t i=0; i<CONFIGURATION.getAttributesNum(); i++) {
		delete hts[i];
	}
	delete []hts;
	printf("Compress completed, account : %u\n", linecount);
}


void testReadIndex() {
	//uint32_t a[20];
	//for(uint32_t i=0; i<20; i++) a[i] = i;
	FILE *file = fopen("./out/index-01", "rb");
	//fread(a, sizeof(uint32_t), 20, file);
	uint8_t a;
	uint32_t b;
	for(int i=0; i<10; i++) {
		fread(&a, sizeof(uint8_t), 1, file);
		fseek(file, (i+1)*sizeof(uint8_t)+i*sizeof(uint32_t), SEEK_SET);
		fread(&b, sizeof(uint32_t), 1, file);
		printf("%u %u\n", a, b);
	}
	FILE *pFile = fopen("a.txt", "r");
	for(uint32_t i=0; i<65536; i++) {

	}
}


void test_print_to_str() {
	uint32_t *a = new uint32_t[3]();
	uint32_t *aa = a;
	uint32_t offset = 0;
	uint32_t *b = new uint32_t[3]{111111,36565,216160};
	for(int i=0; i<3; i++) {
		if(print_to_bit(a, offset, b[i], 22)) {
			a++;
		}
		for(int j=0; j<3; j++) {
			for(int k=31; k>=0; k--) {
				aa[j]&(1<<k) ? putchar('1') : putchar('0');
				if((j*32+(32-k))%22 == 0) putchar(' ');
			}
		}
		puts("");
	}
}

*/
