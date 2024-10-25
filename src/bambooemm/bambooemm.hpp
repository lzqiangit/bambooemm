#ifndef BAMBOOEMM_H_
#define BAMBOOEMM_H_

#include "bamboofilter.hpp"
#include "KV.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>
#include <random>
using namespace std;

class BambooEMM
{

private:
    BambooFilter *bf;
    unsigned char *KI;
    char *value;
    int max_volume, elem_num;
    char *password;
    

public:
    BambooEMM(){
    }
    ~BambooEMM()
    {
    }

    bool Setup(int split_condition_param, int n, int l, char *password)
    {
        this->password = password;
        uint64_t volumn = n > 8192 ? n : 8192;
        bf = new BambooFilter(upperpower2(volumn), split_condition_param);
        elem_num = n;
        max_volume = l;
        return true;
    }

    bool LoadMM(vector<KV *> mm)
    {
        for (KV *kv : mm)
        {
            uint32_t id = get_value_id(kv->value);
            delete kv->value;
            kv->value = new char[BYTE_PER_VALUE];
            memcpy(kv->value, &id, BYTE_PER_VALUE);
            Insert(kv);
        }
        return true;
    }

    bool Insert(KV *kv)
    {
        uint32_t seg_index, bucket_index, tag;

        uint32_t hash_key = BOBHash::run(kv->key, strlen(kv->key), 3);
        char *key_counter = SpliceKey(hash_key, kv->counter);
        char *kvc = SpliceValue(kv);
        if (strlen(kvc) > 32) {
            cout << "<ERROR> key||value||counter 拼接长度超过32!" << endl;
        }
        char *enc_kvc = new char[BYTE_PER_VALUE];
        memset(enc_kvc, 0, BYTE_PER_VALUE);
        int encLen;
        if( -1 == aes_encrypt_string(password, kvc, strlen(kvc), enc_kvc, &encLen) ) {
            cout << "<ERROR> key||value||counter 加密失败!" << endl;
        }      
        if (encLen != BYTE_PER_VALUE) {
            cout << "<ERROR> 密文长度错误!" << endl; 
        }

        bool ret = bf->Insert(key_counter, enc_kvc);
        delete []key_counter;
        delete []kvc;
        delete []enc_kvc;
        return ret;
    }

    /**
     * query前是否需要加密？
     */
    vector<char *> Query(const char *key)
    {
        vector<char *> ret;
        char *temp;
        char *padding_value = new char[BYTE_PER_VALUE];
        uint32_t padding_num = 666666;
        memcpy(padding_value, &padding_num, BYTE_PER_VALUE);
        uint32_t seg_index, bucket_index, tag;
        uint32_t hashKey = BOBHash::run(key, strlen(key), 3);
        for (int i = 0; i < max_volume; i++)
        {
            char *hashKey_counter = SpliceKey(hashKey, i);
            bf->Lookup(hashKey_counter, ret);
            // char *hashKey_counter = Join(hashKey, i);
            // if (bf->Lookup(hashKey_counter, temp))
            // {
            //     ret.push_back(temp);
            // }
            // } else {
            //     ret.push_back(padding_value);
            // }
        }
        return ret;
    }

    BambooFilter *getEMM()
    {
        return bf;
    }

private:
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

    char *SpliceValue(KV *kv, int retRandom = 0){

        string keyStr = kv->key;
        string valueStr = kv->value;

        int len = keyStr.length() + valueStr.length() + LenOfInt(kv->counter) + 2;
        int padLen = 0;

        string ret = keyStr + '|';
        if (len <= 13) {
            padLen = 17 - len;
            char *padCStr = new char[padLen + 1];
            memset(padCStr, '0', padLen);
            memset(padCStr + padLen, 0, 1);
            string padStr = padCStr;
            ret = ret + padStr;
        } 
        ret = ret + to_string(kv->counter) + "|" +valueStr + "|" + RandomNumStr(RANDOM_NUM_LEN, retRandom);
        int retLen = ret.length();
        char *retCStr = new char[retLen + 1];
        memset(retCStr, 0, retLen + 1);
        memcpy(retCStr, (char *)ret.c_str(), retLen);
        return retCStr;
    }

    void ResolveValue(char *spliceValue, char *&key, int &counter, char *&value, int &random) {
        string spliceValueStr = spliceValue;
        int star = 0;
        int end = spliceValueStr.find('|');
        string substring = spliceValueStr.substr(star, end - star);
        key = copy_const_str(substring.c_str());

        star = end + 1;
        end = spliceValueStr.find('|', star);
        char* counterCStr = copy_const_str(spliceValueStr.substr(star, end - star).c_str());
        counter = atoi(counterCStr);

        star = end + 1;
        end = spliceValueStr.find('|', star);
        value = copy_const_str(spliceValueStr.substr(star, end - star).c_str());
        
        star = end + 1;
        end = spliceValueStr.find('|', star);
        char* randomCStr = copy_const_str(spliceValueStr.substr(star, end - star).c_str());
        random = atoi(randomCStr);
    }

    char* RandomNumStr(int len, int pre) {
        int next = pre;
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(pow(10, len-1), pow(10, len) - 1);

        do {
            next = dis(gen);
        } while (pre == next); 
        char *nextStr = new char[len + 1];
        memset(nextStr, 0, len + 1);
        sprintf(nextStr, "%d", next);
        return nextStr;
    }
};

#endif
