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

public:
    BambooEMM()
    {
    }
    ~BambooEMM()
    {
    }

    bool Setup(int level, int n, int l)
    {

        bf = new BambooFilter(upperpower2(12500), 2);
        KI = LoadKey();
        elem_num = n;
        max_volume = l;
        return true;
    }

    bool LoadMM(vector<KV *> mm) {
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
        uint32_t hashKey = BOBHash::run(kv->key, strlen(kv->key), 3);
        char *hashKey_counter = Join(hashKey, kv->counter);
        return bf->Insert(hashKey_counter, kv->value);
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
            char *hashKey_counter = Join(hashKey, i);
            if (bf->Lookup(hashKey_counter, temp))
            {
                ret.push_back(temp);
            }
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
    char *Join(uint32_t key, int counter)
    {
        // cout << key << endl;
        int lenKey = LenOfUInt(key);
        char *keyCStr = new char[lenKey + 1];
        memset(keyCStr, 0, lenKey + 1);
        sprintf(keyCStr, "%u", key);

        int lenTail = LenOfInt(counter) + 1;
        char *tailCStr = new char[lenTail + 1];
        memset(tailCStr, 0, lenTail);
        sprintf(tailCStr, ",%d", counter);

        char *ret = new char[lenKey + lenTail + 1];
        memset(ret, 0, lenKey + lenTail + 1);
        memcpy(ret, keyCStr, lenKey);
        memcpy(ret + lenKey, tailCStr, lenTail);

        delete keyCStr;
        delete tailCStr;
        return ret;
    }

    BambooFilter *getEMM()
    {
        return bf;
    }
};

#endif
