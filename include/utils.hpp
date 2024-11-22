#ifndef _UTILS_H_
#define _UTILS_H_


#include <iostream>
#include <cstring>
#include <vector>
#include <immintrin.h>
#include <mysql/mysql.h>
#include <algorithm>
#include <string.h>
#include <fstream>
#include <sstream>  
#include <string>  
#include <stdexcept> 
#include <bitset>  
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h> 
#include "KV.hpp"
#include "BOBHash.h"
using namespace std;


using std::vector;

#define AES_BLOCK_SIZE 16  //  
#define AES_KEY_LENGTH 16
#define AES_KEY_BITS_LENGTH AES_KEY_LENGTH * 8

typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;


/**
 * 
 */
int LenOfInt(int num);
int LenOfUInt(uint32_t num);
unsigned char* ItoUCStr(int num);

void GenKey(int level);
char* LoadKey();
vector<int> LoadVolumn();

void printBinary(char* data, size_t length);
uint32_t get_value_id(const char* value);
void print_uint64(uint64_t num);
void print_64title();

char* copy_const_str(const char* cstr);

int aes_encrypt_string(char *_pPassword, char *_pInput, int _InLen, char *_pOutBuf, int *_pOutLen);
int aes_decrypt_string(char *_pPassword, char *_pInput, int _InLen, char *_pOutBuf, int *_pOutLen);

vector<KV *> LoadKVList(int &n, int &l);

/** 通过x和st[label][1]计算y,用于定位元素在EMMu中的位置 */
uint32_t GetYHash(uint32_t x, uint32_t st1); 

#endif