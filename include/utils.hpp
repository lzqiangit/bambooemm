#ifndef UTILS_H_
#define UTILS_H_

#include <vector>
#include "KV.hpp"
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <iostream>
#include <cstring>
using namespace std;

# define AES_BLOCK_SIZE 16  //  明文分组的大小
#define AES_KEY_LENGTH 16
#define AES_KEY_BITS_LENGTH AES_KEY_LENGTH * 8


vector<KV *> LoadKVList();
/**
 * 生成随机AES密钥
 */
unsigned char* AESGen(int keyLength);
unsigned char* AESEnc(const unsigned char *key, const unsigned char *plaintext);
unsigned char* AESDec(const unsigned char *key, const unsigned char *ciphertext);

#endif