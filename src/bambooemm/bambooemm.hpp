#ifndef BAMBOOEMM_H_
#define BAMBOOEMM_H_

#include "bamboofilter.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>
#include <map>
#include "xxhash.h"
#include "filterposition.hpp"
#include "UpdateEntry.hpp"

#include <vector>
#include <unordered_map>
#include "Timer.hpp"
#include "KV.hpp"

#define EMMU_SIZE 4096

class BambooEMM
{

private:
    BambooFilter *bf; // 需要统计
    uint32_t max_volume, elem_num;
    bool isEnc = false;
    UpdateEntry *EMMu[EMMU_SIZE]; // EMMu    需要统计

public:
    BambooEMM()
    {
        for (int i = 0; i < EMMU_SIZE; i++)
        {
            EMMu[i] = nullptr;
        }
    }

    ~BambooEMM()
    {
        for (int i = 0; i < EMMU_SIZE; i++)
        {
            if (EMMu[i] != nullptr)
            {
                delete EMMu[i];
            }
        }
    }

    bool Setup(int split_condition_param, int n, uint32_t l)
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
    bool SetupInsert(KV *kv, uint32_t K)
    {

        uint32_t seg_index, bucket_index, tag;

        char *key_counter = kv->QueryKey(K);
        char *kcv = kv->Splice(); // 现在kvc没有长度限制了!
        ValueEntry valueE;
        bool ret;
        if (bf->Lookup(key_counter, valueE))
        {
            // 找到了
            ret = bf->SetupAppend(key_counter, kcv);
        }
        else
        {
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
    vector<ValueEntry> Query(string hashKey)
    {
        vector<ValueEntry> ret;
        uint32_t seg_index, bucket_index, tag;
        for (int i = 0; i < max_volume; i++)
        {
            // cout << "生成加密关键字前:" << Timer::getInstance().getDuration() << "ms" << endl;
            char *hashKey_counter = KV::MakeSearchKey(hashKey, i); // 这个逻辑移动至Client中!!!
            // cout << "生成加密关键字:" << Timer::getInstance().getDuration() << "ms" << endl;
            ValueEntry valueE;
            bf->Lookup(hashKey_counter, valueE);
            // cout << "查询一个关键字:" << Timer::getInstance().getDuration() << "ms" << endl;
            ret.push_back(valueE);
            // break;
        }
        cout << "返回前:" << Timer::getInstance().getDuration() << "ms" << endl;
        return ret;
    }

    /**
     * 传入hashKey
     */
    bool isExistKeyCounter(string hashKey, int counter)
    {
        vector<char *> ret;
        uint32_t seg_index, bucket_index, tag;

        char *hashKey_counter = KV::MakeSearchKey(hashKey, counter);
        ValueEntry valueE;
        return bf->Lookup(hashKey_counter, valueE);
    }

    BambooFilter *getEMM()
    {
        return bf;
    }

    /**
     * 添加随即谁后加密
     */
    void AddRandomAndEncrypt(const char *password)
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
    void ReInsert(char *key, ValueEntry valueE)
    {
        bf->UpdateValue(key, valueE);
    }

    bool Insert(string hashKey, int counter, ValueEntry valueE)
    {
        return bf->Insert(KV::MakeSearchKey(hashKey, counter), valueE);
    }

    void AddUpdata(const uint32_t y, const UpdateEntry ue)
    {
        uint32_t pos = y % EMMU_SIZE;
        if (EMMu[pos] != nullptr)
        {
            cout << "【ERROR】" << "EMMu哈希碰撞!!";
            return;
        }
        EMMu[pos] = new UpdateEntry(ue);
    }

    vector<UpdateEntry> GetUpdataList(uint32_t x, int cnt)
    {
        vector<UpdateEntry> ret;
        int pos;
        for (int i = 0; i < cnt; i++)
        {
            uint32_t y = GetYHash(x, i);
            pos = y % EMMU_SIZE;
            ret.push_back(*(EMMu[pos]));
            // 查找后就清空
            delete EMMu[pos];
            EMMu[pos] = nullptr;
        }
        return ret;
    }

    void ChangeMaxVolume(int new_max_volume)
    {
        this->max_volume = new_max_volume;
    }

    uint32_t getMaxVolume()
    {
        return this->max_volume;
    }

    void setMaxVolume(uint32_t newVolume)
    {
        this->max_volume = newVolume;
    }

    size_t getMemOverhead()
    {

        size_t mySize = 0;

        cout << "-----------------------------Server-------------------------------" << endl;
        // Emmu指针
        size_t emmUMemSize = sizeof(UpdateEntry *) * EMMU_SIZE;
        cout << "emmu(指针):" << getMemSizeStr(emmUMemSize) << endl;
        mySize += emmUMemSize;
        // updata
        size_t upMemSize = 0;
        for (int i = 0; i < EMMU_SIZE; i++)
        {
            if (EMMu[i] != nullptr)
            {
                upMemSize += EMMu[i]->getMemOverhead();
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

};

#endif
