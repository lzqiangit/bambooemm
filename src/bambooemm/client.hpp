#ifndef CLIENT_H_
#define CLIENT_H_

#include "bambooemm.hpp"
#include "utils.hpp"
#include "predefine.h"
#include "filterposition.hpp"

#include <unordered_map>
class Client
{
private:
    /* data */
    int n;
    BambooEMM *bemm;

public:
    Client(/* args */);
    ~Client();

    void SetupEMM(vector<KV *> kvList, int n, int l);
    void MappingStep(vector<KV *> kvList, int l);
    BambooEMM *getBEMM();
    /**
     * 传入需要加密上传的kv,修改value后的随机数,重新上传到filter并更新对应位置的value
     */
    void ReEncrypt(vector<KV *> kvList, char *key = nullptr);

    char *EncValue(char *kvcr);
};

Client::Client(/* args */)
{
}

Client::~Client()
{
}

/**
 * 传入初始数据的EMM，以及kv总数和最大最大容量l
 */
void Client::SetupEMM(vector<KV *> kvList, int n, int l)
{
    this->n = n;
    vector<KV *> maxCounterKVList; // 存储每个key中counter最大的元素
    this->bemm = new BambooEMM();
    this->bemm->Setup(2, MIN_STAR_CAP, l, LoadKey());
    char *tempKey = kvList.at(0)->key;
    for (int i = 0; i < kvList.size(); i++)
    {
        this->bemm->Insert(kvList.at(i)); // counter必须从0开始而且连续 yes
        if (i == kvList.size() - 1)
        {
            maxCounterKVList.push_back(kvList.at(i));
            continue;
        }
        if (strcmp(tempKey, kvList.at(i + 1)->key) != 0)
        {
            maxCounterKVList.push_back(kvList.at(i));
            tempKey = kvList.at(i + 1)->key;
        }
    }
    MappingStep(maxCounterKVList, l);
    // 加密
    bemm->Encrypt(LoadKey());
}

/**
 * 考虑到先插入后填充,或许可以另开一个填充函数
 */
void Client::MappingStep(vector<KV *> kvList, int l)
{
    int counter = 0;
    int passCounter = 0;
    char *key;
    for (KV *kv : kvList)
    {
        counter = kv->counter;
        key = kv->key;
        for (int i = counter; i < l; i++)
        {
            KV *kv = new KV(key, "00000", ++counter);
            if (!(this->bemm->isExistKeyCounter(kv->key, kv->counter)))
            {
                this->bemm->Insert(kv);
            }
            else
            {
                // cout << "OKKKKKKKKKKKK! : " << kv->key << "||" << kv->counter << endl;
                ++passCounter;
            }
            // delete kv;            // ? 泄露?????
        }
    }
    cout << "共用填充:" << passCounter << "|" << 228601 << "(" << (float)passCounter / 228601.f * 100.f << "%)" << endl;
}

BambooEMM *Client::getBEMM()
{
    return this->bemm;
}

/**
 * 查询获得了 : key || counter || value || random ——> hash(hash(key)||counter) -> new splic(value)
 * 客户端重新解析生成 KV，装入kvList
 * 在做了必要的修改之后，将修改后的kvList传入此函数
 * 此函数会拼接 splic_key = hash(hash(key)||counter) 和 splic_val = splic(value),这里splic_val的random值会修改      // ? 在kv中存储random y
 * 然后通过splic_key的值作为key去组合value，传递给服务端bambooemm进行UpdateValue    // char* 转 string作为key，否则无法正常比较值，而是比较char*的指针地址
 */
void Client::ReEncrypt(vector<KV *> kvList, char *keyA = nullptr)
{
    map<FilterPosition, vector<char *>> updateMap;          // 使用unordered_map减小开销，，
    BambooFilter *bf = bemm->getEMM();
    for (KV *kv : kvList)
    {

        char *key = SpliceKey(BOBHash::run(kv->key, strlen(kv->key), 3), kv->counter);
        char *kvcr = SpliceValue(kv, kv->random);
        char *encKvcr = EncValue(kvcr);

        uint32_t seg_index, bucket_index, tag;

        bf->GenerateIndexTagHash(key, seg_index, bucket_index, tag);

        FilterPosition tempFP(seg_index, bucket_index, tag);
        for (auto e : updateMap) {
            if (tempFP == e.first) {
                updateMap[e.first].push_back(encKvcr);
                continue;
            }
        }
        vector<char*> tv;
        tv.push_back(encKvcr);
        updateMap.insert(pair<FilterPosition, vector<char *>>(tempFP, tv));           
    }


    bemm->ReInsert(updateMap);
}

char *Client::EncValue(char *kvcr)
{
    char *enc_value = new char[BYTE_PER_VALUE];
    int encLen = 0;

    memset(enc_value, 0, BYTE_PER_VALUE);

    int len = strlen(kvcr);
    if (-1 == aes_encrypt_string(LoadKey(), kvcr, len, enc_value, &encLen))
    {
        cout << "加密失败!" << endl;
    }
    if (encLen != BYTE_PER_VALUE)
    {
        cout << "密文长度错误!" << endl;
    }

    return enc_value;
}

//

#endif