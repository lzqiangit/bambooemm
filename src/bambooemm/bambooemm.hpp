#ifndef BAMBOOEMM_H_
#define BAMBOOEMM_H_

#include "bamboofilter.hpp"
#include "KV.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>
#include "keyvaluetools.hpp"
#include <map>
#include "filterposition.hpp"

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
    BambooEMM()
    {
    }
    ~BambooEMM()
    {
    }

    bool Setup(int split_condition_param, int n, int l, char *password);
    bool LoadMM(vector<KV *> mm);
    bool Insert(KV *kv);
    /**
     * query前是否需要加密？
     */
    vector<char *> Query(const char *key);
    bool isExistKeyCounter(char *key, int counter);
    BambooFilter *getEMM();
    void Encrypt(char *password);
    /**
     * 传入需要重新插入的kvList
     * 将列表中的value重新填充到对应位置
     */
    void ReInsert(map<FilterPosition, vector<char *>> updateMap);
};

bool BambooEMM::Setup(int split_condition_param, int n, int l, char *password)
{
    this->password = password;
    // uint64_t volumn = n > 8192 ? n : 8192;
    bf = new BambooFilter(upperpower2(n), split_condition_param);
    elem_num = n;
    max_volume = l;
    return true;
}

bool BambooEMM::LoadMM(vector<KV *> mm)
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
bool BambooEMM::Insert(KV *kv)
{
    uint32_t seg_index, bucket_index, tag;

    uint32_t hash_key = BOBHash::run(kv->key, strlen(kv->key), 3);
    char *key_counter = SpliceKey(hash_key, kv->counter);
    char *kvc = SpliceValue(kv);
    if (strlen(kvc) > 32)
    {
        cout << "<ERROR> key||value||counter 拼接长度超过32!" << endl;
    }
    bool ret = bf->Insert(key_counter, kvc);
    delete[] key_counter;
    delete[] kvc;
    return ret;
}

vector<char *> BambooEMM::Query(const char *key)
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

bool BambooEMM::isExistKeyCounter(char *key, int counter)
{
    vector<char *> ret;
    uint32_t seg_index, bucket_index, tag;
    uint32_t hashKey = BOBHash::run(key, strlen(key), 3);

    char *hashKey_counter = SpliceKey(hashKey, counter);
    bf->Lookup(hashKey_counter, ret);
    return ret.size() > 0;
}

BambooFilter *BambooEMM::getEMM()
{
    return bf;
}

void BambooEMM::Encrypt(char *password)
{
    if (isEnc)
    {
        cout << "重复加密！" << endl;
    }
    // 加密
    bf->Encrypt(password);
    isEnc = true;
}

/**
 * 
 */
void BambooEMM::ReInsert(map<FilterPosition, vector<char *>> updateMap)
{

    for (const auto &e : updateMap) {
        FilterPosition fp = e.first;
        vector<char*> vals = e.second;
        bf->UpdateValue(fp, vals);
    }
}

#endif
