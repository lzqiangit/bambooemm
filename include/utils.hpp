#ifndef UTILS_H_
#define UTILS_H_

#include <vector>
#include "KV.hpp"
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <iostream>
#include <cstring>
#include <immintrin.h>
using namespace std;

# define AES_BLOCK_SIZE 16  //  明文分组的大小
#define AES_KEY_LENGTH 16
#define AES_KEY_BITS_LENGTH AES_KEY_LENGTH * 8


vector<KV *> LoadKVList(int &n, int &l);
/**
 * 生成随机AES密钥
 */
int LenOfInt(int num);
int LenOfUInt(uint32_t num);
unsigned char* ItoUCStr(int num);

void GenKey(int level);
char* LoadKey();

void printBinary(char* data, size_t length);
uint32_t get_value_id(const char* value);
void print_uint64(uint64_t num);
void print_64title();

int aes_encrypt_string(char *_pPassword, char *_pInput, int _InLen, char *_pOutBuf, int *_pOutLen);
int aes_decrypt_string(char *_pPassword, char *_pInput, int _InLen, char *_pOutBuf, int *_pOutLen);

#endif