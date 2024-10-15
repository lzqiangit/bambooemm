#ifndef BAMBOOEMM_H_
#define BAMBOOEMM_H_

#include "bamboofilter.hpp"
#include "KV.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>
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
    BambooEMM()
    {
        password = LoadKey();
    }
    ~BambooEMM()
    {
    }

    bool Setup(int level, int n, int l)
    {
        cout << "password: " << password << endl;
        bf = new BambooFilter(upperpower2(12500), 2);
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

    char *SpliceValue(KV *kv){

        string keyStr = kv->key;
        string valueStr = kv->value;

        int len = keyStr.length() + valueStr.length() + LenOfInt(kv->counter) + 2;
        int padLen = 0;

        string ret = keyStr + '|';
        if (len <= 16) {
            padLen = 17 - len;
            char *padCStr = new char[padLen + 1];
            memset(padCStr, '0', padLen);
            memset(padCStr + padLen, 0, 1);
            string padStr = padCStr;
            ret = ret + padStr;
        } 
        ret = ret + to_string(kv->counter) + "|" +valueStr;
        int retLen = ret.length();
        char *retCStr = new char[retLen + 1];
        memset(retCStr, 0, retLen + 1);
        memcpy(retCStr, (char *)ret.c_str(), retLen);
        return retCStr;
    }

    BambooFilter *getEMM()
    {
        return bf;
    }
};

#endif
