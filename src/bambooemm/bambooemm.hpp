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
    //bool LoadMM(vector<KV *> mm);
    bool SetupInsert(KV *kv);
    /**
     * query前是否需要加密？
     */
    vector<ValueEntry> Query(const char *key);
    bool isExistKeyCounter(char *key, int counter);
    BambooFilter *getEMM();
    void Encrypt(char *password);
    /**
     * 用于查询操作融合更新时, 重新对key对于的valueEntry复制
     */
    void ReInsert(char* key, ValueEntry valueE);
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

/**
 * 用于测试?
 */
// bool BambooEMM::LoadMM(vector<KV *> mm)
// {
//     for (KV *kv : mm)
//     {
//         uint32_t id = get_value_id(kv->value);
//         delete kv->value;
//         kv->value = new char[BYTE_PER_VALUE];
//         memcpy(kv->value, &id, BYTE_PER_VALUE);
//         SetupInsert(kv);
//     }
//     return true;
// }

/**
 * 用于初始化时调用, 此时为明文状态
 */
bool BambooEMM::SetupInsert(KV *kv)
{
    if (kv->value == 39410) {
        cout << endl;
    }
    uint32_t seg_index, bucket_index, tag;

    uint32_t hash_key = BOBHash::run(kv->key, strlen(kv->key), 3);
    char *key_counter = SpliceKey(hash_key, kv->counter);
    char *kvc = SpliceValue(kv);    // 现在kvc没有长度限制了!
    ValueEntry valueE;
    bool ret;
    if (bf->Lookup(key_counter, valueE)) {
        // 找到了
        ret = bf->SetupAppend(key_counter, kvc);
    } else {
        valueE.SetValue(strlen(kvc) + 1, kvc);
        ret = bf->Insert(key_counter, valueE);
    }
     
    delete[] key_counter;
    delete[] kvc;
    return ret;
}

vector<ValueEntry> BambooEMM::Query(const char *key)
{
    vector<ValueEntry> ret;
    uint32_t seg_index, bucket_index, tag;
    uint32_t hashKey = BOBHash::run(key, strlen(key), 3);
    for (int i = 0; i < max_volume; i++)
    {
        char *hashKey_counter = SpliceKey(hashKey, i);
        ValueEntry valueE;
        bf->Lookup(hashKey_counter, valueE);
        ret.push_back(valueE);

    }
    return ret;
}

bool BambooEMM::isExistKeyCounter(char *key, int counter)
{
    vector<char *> ret;
    uint32_t seg_index, bucket_index, tag;
    uint32_t hashKey = BOBHash::run(key, strlen(key), 3);

    char *hashKey_counter = SpliceKey(hashKey, counter);
    ValueEntry valueE;
    return bf->Lookup(hashKey_counter, valueE);
    
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
void BambooEMM::ReInsert(char* key, ValueEntry valueE)
{

    bf->UpdateValue(key, valueE);

}

#endif
