#ifndef _KEY_VALUE_TOOLS_H_
#define _KEY_VALUE_TOOLS_H_

#include "utils.hpp"
#include "predefine.h"
#include <iostream>
#include <string>
#include <random>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#define RAND_LEN 2

int aes_encrypt_string(char *_pPassword, char *_pInput, int _InLen, char *_pOutBuf, int *_pOutLen);
int aes_decrypt_string(char *_pPassword, char *_pInput, int _InLen, char *_pOutBuf, int *_pOutLen);

char *SpliceKey(uint32_t key, int counter);
char *SpliceValue(KV *kv, int retRandom = 0);
void ResolveValue(char *spliceValue, char *&key, int &counter, char *&value, int &random);
char *RandomNumStr(int len, int pre);


/*
*****************************************************************************************
*   函 数 名: aes_encrypt_string
*   功能说明: AES加密字符串
*   形    参:   _pPassword  :   密码
*               _pInput     :   输入数据
*               _InLen      :   输入数据长度
*               _pOutBuf    :   输出AES编码数据
*               _pOutLen    :   输出AES编码数据长度
*   返 回 值: 0：成功, -1：失败
*   注意: 传入的指针需要提前申请空间,否则会报Segmentation fault
*****************************************************************************************
*/
int aes_encrypt_string(char *_pPassword, char *_pInput, int _InLen, char *_pOutBuf, int *_pOutLen)
{
    // 上下文结构
    EVP_CIPHER_CTX *pEn_ctx = NULL;

    int ret = -1;
    int flen = 0, outlen = 0;
    int i, nrounds = 1;

    // 存储秘钥和初始化向量
    unsigned char key[32] = {};
    unsigned char iv[32] = {};

    // 参数判断
    if (_pPassword == NULL || _pInput == NULL || _pOutBuf == NULL || _pOutLen == NULL)
    {
        return ret;
    }

    // 设置使用 256 位密钥长度的 AES 加密算法，并采用 CBC 模式。
    const EVP_CIPHER *cipherType = EVP_aes_256_cbc();
    if (cipherType == NULL)
    {
        goto clean;
    }

    /*
     * Gen key & IV for AES 256 CBC mode. A SHA1 digest is used to hash the supplied key material.
     * nrounds is the number of times the we hash the material. More rounds are more secure but
     * slower.
     */
    // 通过输入密码产生密钥key和初始化向量iv
    i = EVP_BytesToKey(cipherType, EVP_md5(), NULL, (unsigned char *)_pPassword, strlen(_pPassword), nrounds, key, iv);
    if (i != 32)
    {
        printf("Key size is %d bits - should be 256 bits\n", i);
        goto clean;
    }

    pEn_ctx = EVP_CIPHER_CTX_new();                         // 创建加密上下文
    EVP_CIPHER_CTX_init(pEn_ctx);                           // 初始化 EVP_CIPHER_CTX 上下文
    EVP_EncryptInit_ex(pEn_ctx, cipherType, NULL, key, iv); // 初始化加密操作

    /* Update cipher text */
    if (!EVP_EncryptUpdate(pEn_ctx, (unsigned char *)_pOutBuf, &outlen, (unsigned char *)_pInput, _InLen))
    { // 处理数据
        cout << "Error,ENCRYPR_UPDATE:" << endl;
        goto clean;
    }

    /* updates the remaining bytes */
    if (!EVP_EncryptFinal_ex(pEn_ctx, (unsigned char *)(_pOutBuf + outlen), &flen))
    { // 完成加密操作，处理剩余字节
        cout << "Error,ENCRYPT_FINAL:" << endl;
        goto clean;
    }

    *_pOutLen = outlen + flen;

    ret = 0; /* SUCCESS */

clean:
    // 清理内存
    if (pEn_ctx)
        EVP_CIPHER_CTX_cleanup(pEn_ctx);
    if (pEn_ctx)
        EVP_CIPHER_CTX_free(pEn_ctx);

    return ret;
}

/*
*****************************************************************************************
*   函 数 名: aes_decrypt_string
*   功能说明: AES解密得到字符串
*   形    参:   _pPassword  :   密码
*               _pInput     :   输入需解密的数据
*               _InLen      :   输入需解密的数据长度
*               _pOutBuf    :   输出AES解密后的字符串
*               _pOutLen    :   输出AES编码数据长度
*   返 回 值: 0：成功, -1：失败
*****************************************************************************************
*/
int aes_decrypt_string(char *_pPassword, char *_pInput, int _InLen, char *_pOutBuf, int *_pOutLen)
{
    // 上下文结构
    EVP_CIPHER_CTX *pDe_ctx = NULL;

    int ret = -1;
    int flen = 0, outlen = 0;
    int i, nrounds = 1;

    // 存储秘钥和初始化向量
    unsigned char key[32] = {};
    unsigned char iv[32] = {};

    // 参数判断
    if (_pPassword == NULL || _pInput == NULL || _pOutBuf == NULL || _pOutLen == NULL)
    {
        return ret;
    }

    // 设置使用 256 位密钥长度的 AES 加密算法，并采用 CBC 模式。
    const EVP_CIPHER *cipherType = EVP_aes_256_cbc();
    if (cipherType == NULL)
    {
        goto clean;
    }

    /*
     * Gen key & IV for AES 256 CBC mode. A SHA1 digest is used to hash the supplied key material.
     * nrounds is the number of times the we hash the material. More rounds are more secure but
     * slower.
     */
    // 通过输入密码产生密钥key和初始化向量iv
    i = EVP_BytesToKey(cipherType, EVP_md5(), NULL, (unsigned char *)_pPassword, strlen(_pPassword), nrounds, key, iv);
    if (i != 32)
    {
        printf("Key size is %d bits - should be 256 bits\n", i);
        goto clean;
    }

    pDe_ctx = EVP_CIPHER_CTX_new();                         // 创建加密上下文
    EVP_CIPHER_CTX_init(pDe_ctx);                           // 初始化 EVP_CIPHER_CTX 上下文
    EVP_DecryptInit_ex(pDe_ctx, cipherType, NULL, key, iv); // 初始化解密操作

    /* Update cipher text */
    if (!EVP_DecryptUpdate(pDe_ctx, (unsigned char *)_pOutBuf, &outlen, (unsigned char *)_pInput, _InLen))
    { // 处理数据
        cout << "Error,DEC_UPDATE:" << _pOutBuf << endl;
        goto clean;
    }

    /* updates the remaining bytes */
    if (EVP_DecryptFinal_ex(pDe_ctx, (unsigned char *)(_pOutBuf + outlen), &flen) != 1)
    { // 完成解密操作，处理剩余字节
        cout << "Error,DEC_FINAL!" << _pOutBuf << "|" << flen << endl;
        goto clean;
    }

    *_pOutLen = outlen + flen;

    ret = 1;
clean:
    // 清理内存
    if (pDe_ctx)
        EVP_CIPHER_CTX_cleanup(pDe_ctx);
    if (pDe_ctx)
        EVP_CIPHER_CTX_free(pDe_ctx);

    return ret;
}

/**
 * 将key同counter拼接，返回拼接后的字符串
 * 注意不会将key哈希
 */
char *SpliceKey(uint32_t key, int counter)
{
    string keyStr = to_string(key);
    string counterStr = to_string(counter);
    string keyCounterStr = keyStr + "|" + counterStr;
    char *retCStr = new char[keyCounterStr.length() + 1];
    memset(retCStr, 0, keyCounterStr.length() + 1);
    memcpy(retCStr, (char *)keyCounterStr.c_str(), keyCounterStr.length());
    return retCStr;
}

/**
 * 返回的长度为segment中value单元的规定值
 */
char *SpliceValue(KV *kv, int retRandom = 0)
{

    string keyStr = kv->key;
    string valueStr = kv->value;

    int len = keyStr.length() + valueStr.length() + LenOfInt(kv->counter) + 2;
    int padLen = 0;

    string ret = keyStr + '|';
    if (len <= 13)
    {
        padLen = 17 - len - RAND_LEN;
        char *padCStr = new char[padLen + 1];
        memset(padCStr, '0', padLen);
        memset(padCStr + padLen, 0, 1);
        string padStr = padCStr;
        ret = ret + padStr;
    }
    ret = ret + to_string(kv->counter) + "|" + valueStr + "|" + RandomNumStr(RANDOM_NUM_LEN, retRandom);
    int retLen = ret.length();
    char *retCStr = new char[BYTE_PER_VALUE];
    memset(retCStr, 0, BYTE_PER_VALUE);
    memcpy(retCStr, (char *)ret.c_str(), retLen);
    return retCStr;
}

void ResolveValue(char *spliceValue, char *&key, int &counter, char *&value, int &random)
{
    string spliceValueStr = spliceValue;
    int star = 0;
    int end = spliceValueStr.find('|');
    string substring = spliceValueStr.substr(star, end - star);
    key = copy_const_str(substring.c_str());

    star = end + 1;
    end = spliceValueStr.find('|', star);
    char *counterCStr = copy_const_str(spliceValueStr.substr(star, end - star).c_str());
    counter = atoi(counterCStr);

    star = end + 1;
    end = spliceValueStr.find('|', star);
    value = copy_const_str(spliceValueStr.substr(star, end - star).c_str());

    star = end + 1;
    end = spliceValueStr.find('|', star);
    char *randomCStr = copy_const_str(spliceValueStr.substr(star, end - star).c_str());
    random = atoi(randomCStr);
}

char *RandomNumStr(int len, int pre)
{
    int next = pre;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(pow(10, len - 1), pow(10, len) - 1);

    do
    {
        next = dis(gen);
    } while (pre == next);
    char *nextStr = new char[len + 1];
    memset(nextStr, 0, len + 1);
    sprintf(nextStr, "%d", next);
    return nextStr;
}

#endif