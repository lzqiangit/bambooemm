#ifndef KV_H_
#define KV_H_


#include "BOBHash.h"
#include <string>
#include <cstring>
#include <vector>
#include "Timer.hpp"
#include <iostream>

#include <xxhash.h>

typedef unsigned int uint32_t;
using std::string;
using std::__cxx11::to_string;
using std::vector;
using std::cout;


class KV
{
public:
    char *key;
    char *value;
    int counter;

    KV() {
        key = nullptr;
        value = nullptr;
        counter = 0;
    }

    /**
     * 构建一个KV，传入key,counter和value
     * 注意构造kv后后对key和value的释放
     */
    KV(const char* key, int counter, const char* value) {
        this->key = strdup(key);
        this->value = strdup(value);
        this->counter = counter;
    }

    /**
     * 构建一个填充KV
     */
    KV(const char *key, int counter) {
        this->key = strdup(key);
        this->counter = counter;
        this->value = nullptr;
        BePadding();
    }

    ~KV()
    {
        delete key;
        delete value;
    }

    /**
     * 从key|counter|val的字符串中导入kv
     * 首先以 | 分割，key为第一个，counter为第二个，value为第三个
     * 注意counter需要转化为整形
     */
    KV(char *kcv) { 
        char *key = strtok(kcv, "|");
        char *counter = strtok(nullptr, "|");
        char *value = strtok(nullptr, "|");
        this->key = strdup(key);
        this->value = strdup(value);
        this->counter = atoi(counter);
    }

    /**
     * 拷贝构造函数
     * 深拷贝
     */
    KV(const KV& others) {
        this->key = strdup(others.key);
        this->value = strdup(others.value);
        this->counter = others.counter;
    }

    KV& operator=(const KV& others) {
        if (this->key != nullptr) {
            delete []this->key;
        }
        if (this->value != nullptr) {
            delete []this->value;
        }
        this->key = strdup(others.key);
        this->value = strdup(others.value);
        this->counter = others.counter;
        return *this;
    }

    /**
     * 将成员变量以 key|counter|value 的形式拼接成字符串
     */
    char *Splice() {
        int keyLen = strlen(this->key);
        int valueLen = strlen(this->value);
        int counterLen = LenOfInt(this->counter);
        int totalLen = keyLen + valueLen + counterLen + 2;
        char *ret = new char[totalLen + 1];
        memset(ret, 0, totalLen + 1);
        memcpy(ret, this->key, keyLen);
        ret[keyLen] = '|';
        // 使用sprintf将counter拼接到ret中
        sprintf(ret + keyLen + 1, "%d", this->counter);
        ret[keyLen + 1 + counterLen] = '|';
        memcpy(ret + keyLen + 1 + counterLen + 1, this->value, valueLen);
        return ret;
    }

    /**
     * 获取其向服务端发送查询请求所需的key <- hash(key)|counter
     */
    char *QueryKey(uint32_t K) {
        return MakeSearchKey(MakeHashKey(this->key, K), this->counter);
    }

    /**
     * 设置为填充值
     */
    void BePadding() {
        if (this->value != nullptr)    delete[] this->value;
        this->value = strdup("P");
    }

    bool isPadding() {
        return (strlen(this->value) == 1 && this->value[0] == 'P'); 
    }

    void setValue(const char *newValue) {
        if (this->value != nullptr) {
            delete[] this->value;
        }
        this->value = strdup(newValue);
    }

    /**
     * 求key的xxHash哈希值
     */
    static string MakeHashKey(const char *key, uint32_t K) {

        //uint32_t hash_key = BOBHash::run(key, strlen(key), 3);
        //cout << "HashBefore:" << Timer::getInstance().getDuration() << "\n";
        uint32_t hash_key = XXH32(key, strlen(key), K);
        //cout << "HashEnd:" << Timer::getInstance().getDuration() << "\n";
        string keyStr = to_string(hash_key);
        return keyStr;
    }
    /** 
     * 传入哈希后的key和counter,输出两者的拼接
     * 配合 KV::MakeHashKey 生成key的哈希值string
     * key <- hash(k)||c
     */
    static char *MakeSearchKey(string hashKey, int c) {
        
        string counterStr = to_string(c);
        string keyCounterStr = hashKey + "|" + counterStr;
        int len = keyCounterStr.length();
        char *retCStr = new char[len + 1];
        memset(retCStr, 0, len + 1);
        memcpy(retCStr, (char *)keyCounterStr.c_str(), len);
        return retCStr;
    }
    /**
     * 从key||counter||val的字符串列表中导入kv的列表
     */
    static vector<KV> LoadKVList(vector<char*> kvStrList) {
        vector<KV> ret;
        for (char * kvStr : kvStrList) {
            KV kv(kvStr);
            ret.push_back(kv);
        }
        return ret;
    }

private:
    char* copy_const_str(const char* cstr) {
        int len = strlen(cstr);
        char* cpy = new char[len + 1];
        memset(cpy, 0, len + 1);
        memcpy(cpy, cstr, len);
        return cpy;
    }
    int LenOfInt(int num) {
        int len = 1;
        while (num >= 10) {
            ++len;
            num /= 10;
        }
        return len;
    }
};

#endif
