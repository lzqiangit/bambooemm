#ifndef BAMBOOEMM_H_
#define BAMBOOEMM_H_

#include "bamboofilter.hpp"
#include "KV.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>
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
    void AddRandomAndEncrypt(char *password);
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

    uint32_t seg_index, bucket_index, tag;

    char *key_counter = kv->QueryKey();
    char *kcv = kv->Splice();    // 现在kvc没有长度限制了!
    ValueEntry valueE;
    bool ret;
    if (bf->Lookup(key_counter, valueE)) {
        // 找到了
        ret = bf->SetupAppend(key_counter, kcv);
    } else {
        valueE.SetValue(strlen(kcv) + 1, kcv);
        ret = bf->Insert(key_counter, valueE);
    }
     
    delete[] key_counter;
    delete[] kcv;
    return ret;
}

vector<ValueEntry> BambooEMM::Query(const char *key)
{
    vector<ValueEntry> ret;
    uint32_t seg_index, bucket_index, tag;
    for (int i = 0; i < max_volume; i++)
    {
        char *hashKey_counter = KV::MakeKey(key, i);
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

    char *hashKey_counter = KV::MakeKey(key, counter);
    ValueEntry valueE;
    return bf->Lookup(hashKey_counter, valueE);
    
}

BambooFilter *BambooEMM::getEMM()
{
    return bf;
}

/**
 * 添加随即谁后加密
 */
void BambooEMM::AddRandomAndEncrypt(char *password)
{
    if (isEnc)
    {
        cout << "重复加密！" << endl;
    }
    bf->AddRandom();
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
