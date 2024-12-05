#ifndef BAMBOOEMM_H_
#define BAMBOOEMM_H_

#include "bamboofilter.hpp"
#include "KV.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>
#include <map>
#include "filterposition.hpp"
#include "UpdataEntry.hpp"
#include <vector>

#define DEFAULT_EMMU_SIZE 4096


class BambooEMM
{

private:
    BambooFilter *bf;
    unsigned char *KI;
    char *value;
    int max_volume, elem_num;
    char *password;
    bool isEnc = false;
    UpdataEntry **updata;    // EMMu
    uint32_t emmUSize;

public:
    BambooEMM()
    {
        this->emmUSize = DEFAULT_EMMU_SIZE;
        updata = new UpdataEntry*[DEFAULT_EMMU_SIZE];
        for (int i=0; i<emmUSize; i++) {
            updata[i] = nullptr;
        }
    }
    BambooEMM(uint32_t emmUSize)
    {
        this->emmUSize = emmUSize;
        updata = new UpdataEntry*[emmUSize];
        for (int i=0; i<emmUSize; i++) {
            updata[i] = nullptr;
        }
    }

    ~BambooEMM()
    {
        delete[] value;
        delete[] bf;
        delete[] KI;
        delete[] password;
        for (int i=0; i<emmUSize; i++) {
            if (updata[i] != nullptr) {
                delete updata[i];
            }
        }
        delete[] updata;
    }

    bool Setup(int split_condition_param, int n, int l, char *password);
    //bool LoadMM(vector<KV *> mm);
    bool SetupInsert(KV *kv);

    
    /**
     * query前是否需要加密？
     */
    vector<ValueEntry> Query(string hashKey);
    bool isExistKeyCounter(string hashKey, int counter);
    BambooFilter *getEMM();
    void AddRandomAndEncrypt(char *password);
    /**
     * 用于查询操作融合更新时, 重新对key对于的valueEntry复制
     */
    void ReInsert(char* key, ValueEntry valueE);
    /**
     * 用于更新操作中插入新的key的值
     * 注意: 传入的valueE需要提前加密
     */
    bool Insert(string hashKey, int counter, ValueEntry valueE);
    /**
     * 用于上传更新
     */
    void AddUpdata(uint32_t y, UpdataEntry ue);

    vector<UpdataEntry> GetUpdataList(uint32_t x, int cnt);
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

/**
 * 传入哈希后的key,返回该key的l给valueEntry
 */
vector<ValueEntry> BambooEMM::Query(string hashKey)
{
    vector<ValueEntry> ret;
    uint32_t seg_index, bucket_index, tag;
    for (int i = 0; i < max_volume; i++)
    {
        char *hashKey_counter = KV::MakeSearchKey(hashKey, i);            // 这个逻辑移动至Client中!!!
        ValueEntry valueE;
        bf->Lookup(hashKey_counter, valueE);
        ret.push_back(valueE);

    }
    return ret;
}
/**
 * 传入hashKey
 */
bool BambooEMM::isExistKeyCounter(string hashKey, int counter)
{
    vector<char *> ret;
    uint32_t seg_index, bucket_index, tag;

    char *hashKey_counter = KV::MakeSearchKey(hashKey, counter);
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

bool BambooEMM::Insert(string hashKey, int counter, ValueEntry valueE) {
    return bf->Insert(KV::MakeSearchKey(hashKey, counter), valueE);
}

void BambooEMM::AddUpdata(uint32_t y, UpdataEntry ue) {
    uint32_t pos = y % emmUSize;
    if (updata[pos] != nullptr) {
        cout << "EMMu哈希碰撞!!";
        exit(-1);
    }
    updata[pos] = new UpdataEntry(ue);
}

vector<UpdataEntry> BambooEMM::GetUpdataList(uint32_t x, int cnt) {
    vector<UpdataEntry> ret;
    int pos;
    for (int i=0; i<cnt; i++) {
        uint32_t y = GetYHash(x, i);
        pos = y % emmUSize;
        ret.push_back( *(updata[pos]) );
        // 查找后就清空
        delete updata[pos];
    }
    return ret;
}

#endif
