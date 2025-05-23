#ifndef _UTILS_H_
#define _UTILS_H_


#include <iostream>
#include <cstring>
#include <vector>
#include <immintrin.h>
#include <mysql/mysql.h>
#include <algorithm>
#include <fstream>
#include <sstream>  
#include <string>  
#include <stdexcept> 
#include <bitset>  
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h> 
#include <ctime>
#include "BOBHash.h"

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::max;
using std::bitset;
using std::ofstream;
using std::to_string;



#define AES_BLOCK_SIZE 16  //  
#define AES_KEY_LENGTH 16
#define AES_KEY_BITS_LENGTH AES_KEY_LENGTH * 8

typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;
using std::pair;

class KV;
/**
 * 
 */
int LenOfInt(int num);
int LenOfUInt(uint32_t num);


void GenKey(int level);
const char* LoadKey();
vector<int> LoadVolumn();

void printBinary(char* data, size_t length);
uint32_t get_value_id(const char* value);
void print_uint64(uint64_t num);
void print_64title();

char* copy_const_str(const char* cstr);

int aes_encrypt_string(const char *_pPassword, const char *_pInput, int _InLen, char *_pOutBuf, int *_pOutLen);
int aes_decrypt_string(const char *_pPassword, const char *_pInput, int _InLen, char *_pOutBuf, int *_pOutLen);

vector<KV *> LoadKVList(int &n, int &l);

/** 通过x和st[label][1]计算y,用于定位元素在EMMu中的位置 */
uint32_t GetYHash(uint32_t x, uint32_t st1); 

// 获取时间戳
uint64_t getTimestamp();
string getMemSizeStr(size_t size);

// 拼接char*字符串
char* concat(char delim, const char* first, ...);
// 
void SaveToCSV(vector<pair<size_t, double>> timeList, string filename, string title);
#endif