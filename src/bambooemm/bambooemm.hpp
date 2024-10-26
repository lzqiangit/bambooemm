#ifndef BAMBOOEMM_H_
#define BAMBOOEMM_H_

#include "bamboofilter.hpp"
#include "KV.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>
#include "keyvaluetools.hpp"

class BambooEMM
{

private:
    BambooFilter *bf;
    unsigned char *KI;
    char *value;
    int max_volume, elem_num;
    char *password;
    bool isEnc = false;
    

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
        // char *enc_kvc = new char[BYTE_PER_VALUE];
        // memset(enc_kvc, 0, BYTE_PER_VALUE);
        // int encLen;
        // if( -1 == aes_encrypt_string(password, kvc, strlen(kvc), enc_kvc, &encLen) ) {
        //     cout << "<ERROR> key||value||counter 加密失败!" << endl;
        // }      
        // if (encLen != BYTE_PER_VALUE) {
        //     cout << "<ERROR> 密文长度错误!" << endl; 
        // }

        bool ret = bf->Insert(key_counter, kvc);
        delete []key_counter;
        delete []kvc;
        //delete []enc_kvc;
        return ret;
    }

    /**
     * query前是否需要加密？
     */
    vector<char *> Query(const char *key)
    {
        vector<char *> ret;
        uint32_t seg_index, bucket_index, tag;
        uint32_t hashKey = BOBHash::run(key, strlen(key), 3);
        for (int i = 0; i < max_volume; i++)
        {
            char *hashKey_counter = SpliceKey(hashKey, i);
            bf->Lookup(hashKey_counter, ret);
        }
        return ret;
    }

    BambooFilter *getEMM()
    {
        return bf;
    }

    void Encrypt(char *password) {
        if (isEnc) {
            cout << "重复加密！" << endl;
        }
        // 加密
        bf->Encrypt(password);
    }

private:
    
};

#endif
