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
#include <unordered_map>
#include "Timer.hpp"

#define DEFAULT_EMMU_SIZE 4096


class BambooEMM
{

private:
    BambooFilter *bf;        // 需要统计
    uint32_t max_volume, elem_num;
    bool isEnc = false;
    UpdataEntry **updata;    // EMMu    需要统计
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
        delete[] bf;
        for (int i=0; i<emmUSize; i++) {
            if (updata[i] != nullptr) {
                delete updata[i];
            }
        }
        delete[] updata;
    }

    bool Setup(int split_condition_param, int n, uint32_t l);
    //bool LoadMM(vector<KV *> mm);
    bool SetupInsert(KV *kv);

    
    /**
     * query前是否需要加密？
     */
    vector<ValueEntry> Query(string hashKey);
    vector<ValueEntry> Query(string hashKey, int l);
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

    void ChangeMaxVolume(int new_max_volume);

    uint32_t getMaxVolume();
    void setMaxVolume(uint32_t newVolume);

    /**
     * 计算存储开销
     * 返回各个部分的存储开销
     * server: server的存储开销
     *      bf
     *      updata
     * 
     */
    size_t getMemOverhead();
};

bool BambooEMM::Setup(int split_condition_param, int n, uint32_t l)
{
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
        cout << "生成加密关键字前:" << Timer::getInstance().getDuration() << "ms" << endl;
        char *hashKey_counter = KV::MakeSearchKey(hashKey, i);            // 这个逻辑移动至Client中!!!
        cout << "生成加密关键字:" << Timer::getInstance().getDuration() << "ms" << endl;
        ValueEntry valueE;
        bf->Lookup(hashKey_counter, valueE);
        cout << "查询一个关键字:" << Timer::getInstance().getDuration() << "ms" << endl;
        ret.push_back(valueE);
        //break;
    }
    return ret;
}

/**
 * 传入哈希后的key,返回该key的l给valueEntry
 */
vector<ValueEntry> BambooEMM::Query(string hashKey, int l)
{
    vector<ValueEntry> ret;
    uint32_t seg_index, bucket_index, tag;
    for (int i = 0; i < l; i++)
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
        updata[pos] = nullptr;
    }
    return ret;
}

void BambooEMM::ChangeMaxVolume(int new_max_volume) {
    this->max_volume = new_max_volume;
}

uint32_t BambooEMM::getMaxVolume() {
    return this->max_volume;
}

void BambooEMM::setMaxVolume(uint32_t newVolume) {
    this->max_volume = newVolume;
}

size_t BambooEMM::getMemOverhead() {
    
    size_t mySize = 0;

    cout << "-----------------------------Server-------------------------------" << endl;
    // Emmu指针
    size_t emmUMemSize = sizeof(UpdataEntry*) * emmUSize;
    cout << "emmu(指针):" << getMemSizeStr(emmUMemSize) << endl;
    mySize += emmUMemSize;
    // updata
    size_t upMemSize = 0;
    for (int i=0; i<emmUSize; i++) {
        if (updata[i] != nullptr) {
            upMemSize += updata[i]->getMemOverhead();
        }
    }
    cout << "updata(元素):" << getMemSizeStr(upMemSize) << endl;
    mySize += upMemSize;
    // 获取bamboo的空间
    size_t bfMemSize = bf->getMemOverhead();
    cout << "Bamboo:" << getMemSizeStr(bfMemSize) << endl;
    mySize += bfMemSize;
    // 获取其他元素
    size_t otherMemSize = sizeof(BambooEMM);
    cout << "others:" << getMemSizeStr(otherMemSize) << endl;
    mySize += otherMemSize;
    // 输出总的
    cout << "Server总空间:" << getMemSizeStr(mySize) << endl;
    return mySize;
}
#endif
