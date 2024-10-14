#include <iostream>
#include "bambooemm.hpp"
#include "KV.hpp"
#include "utils.hpp"
#include "common/random.h"
using namespace std;

void Extend(Segment *src, int act_bit, int table_size)
{
    Segment *dst = new Segment(*src);

    uint32_t num_seg_bits_ = (uint32_t)ceil(log2((double)table_size));
    int num_table_bits_ = num_seg_bits_ + BUCKETS_PER_SEG;
    src->EraseEle(true, act_bit - 1);
    dst->EraseEle(false, act_bit - 1);
}


void extend() {
    BambooEMM bemm;
    uint32_t max_volumn = 30;
    bemm.Setup(3, 1000, max_volumn);
    uint32_t add_count = 273;           // 273
    vector<KV*> data;
    vector<char*> key;
    for (uint32_t i=0; i<add_count; i++) {
        char* temp_key = new char[5];
        memset(temp_key, 0, 5);
        memcpy(temp_key, &i, 4); 
        for (uint32_t j=0; j<max_volumn; j++) {
            char *temp_value = new char[5];
            memset(temp_value, 0, 5);
            uint32_t temp_v = (max_volumn * i) + j;
            memcpy(temp_value, (char*)(&temp_v), 4);
            KV *kv = new KV(temp_key, temp_value, j);
            data.push_back(kv);
        }
        key.push_back(temp_key); 
    }
    
    for (int i=0; i<data.size(); i++) {
        if (i == 1980) {
            cout << endl;
        }
        bemm.Insert(data[i]);
    }

// ²éÑ¯
    uint32_t counter = 0;
    for (int i=0; i<key.size(); i++) {
        vector<char*> ret = bemm.Query(key.at(i));
        cout << i << "\t" ;
        for (int j=0; j<ret.size(); j++) {
            cout << key.at(i) << "|" << ret.at(j) << "|" << j << endl;
            
        }
        cout << endl;
    }

}

char *SpliceValue(KV *kv){

        string keyStr = kv->key;
        string valueStr = kv->value;

        int len = keyStr.length() + valueStr.length() + LenOfInt(kv->counter) + 2;
        int padLen = 0;

        string ret = keyStr + '|' + valueStr + '|';
        if (len <= 16) {
            padLen = 17 - len;
            char *padCStr = new char[padLen + 1];
            memset(padCStr, '0', padLen);
            memset(padCStr + padLen, 0, 1);
            string padStr = padCStr;
            ret = ret + padStr;
        } 
        ret = ret + to_string(kv->counter);
        int retLen = ret.length();
        char *retCStr = new char[retLen + 1];
        memset(retCStr, 0, retLen + 1);
        memcpy(retCStr, (char *)ret.c_str(), retLen);
        return retCStr;
    }

void testValue() {
    int n, l;
    vector<KV*> kvList = LoadKVList(n,l);
    BambooEMM bemm;
    bemm.Setup(3, n/0.75, l);
    cout << n << "|" << l << endl;
    for (KV *kv : kvList) {
        // cout << kv->key << "|" << kv->value << "|" << kv->counter << endl;
        if (strcmp("key_350", kv->key) == 0 && strcmp(kv->value,"4882") == 0) {
            cout << endl;
        }
        bemm.Insert(kv);
    }

    char *password = LoadKey();
    char *dec = new char[32];
    int decLen;
    // query
    int counter = 0;
    for (int i=0; i<1024; i++) {
        string key = "key_" + to_string(i);
        //cout << key << " : ";
        
        vector<char*> values = bemm.Query((char*)key.c_str());
        for (int i=0; i<values.size(); i++) {
            char *value = values.at(i);
            memset(dec, 0, 32);
            aes_decrypt_string(password, value, BYTE_PER_VALUE, dec, &decLen);
            KV *kv = new KV((char*)key.c_str(), (char*)to_string(counter++).c_str(), i);
            char *aim = SpliceValue(kv);
            if (strcmp(aim, dec) != 0) {
                cout << key << ":" << dec << "\t aim: " << aim << endl;
                --counter;
            }
            // cout << dec << " # ";   
        }
        //cout << endl;
    }
}

int main()
{
    testValue();
    return 0;
}
