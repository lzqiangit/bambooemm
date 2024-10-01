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

public:
    BambooEMM()
    {
    }
    ~BambooEMM()
    {
    }

    bool setup(int level, int n, int l, vector<KV *> mm)
    {

        bf = new BambooFilter(upperpower2(200000), 2);
        KI = LoadKey();
        for (KV *kv : mm)
        {

            uint32_t hashKey = BOBHash::run(kv->key, strlen(kv->key), 3);
            char* hashKey_counter = Join(hashKey, kv->counter);
            // cout << kv->key << "|" << hashKey << "|" << hashKey_counter << endl;
            // break;
            insert(hashKey_counter, kv->value);
        }

        return true;
    }

    bool insert(const char* key, const char* value) {

        uint32_t seg_index, bucket_index, tag;

        bf->GenerateIndexTagHash(key, seg_index, bucket_index, tag);
        cout << tag << endl;
        return true;
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
        char* tailCStr = new char[lenTail + 1];
        memset(tailCStr, 0, lenTail);
        sprintf(tailCStr, ",%d", counter);

        char *ret = new char[lenKey + lenTail + 1];
        memset(ret, 0, lenKey+lenTail+1);
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
