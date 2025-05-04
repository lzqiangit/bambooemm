#ifndef TWOCH_CLIENT_H_
#define TWOCH_CLIENT_H_

#include <vector>
#include <xxhash.h>
#include <unordered_map>

#include "KV.hpp"
#include "TwoCH.hpp"
#include "UpdateEntry.hpp"

typedef unsigned int uint32_t;
using std::unordered_map;
using std::vector;

class TwochClient
{
private:
    const uint32_t mK = 123456;  // 计算关键字的哈希值的种子
    const uint32_t mKu = 654321; // 计算
    const char *mKEnc = "135790";

    TwoCH *mTwoch;                                    // 主存储结构
    unordered_map<string, vector<KV>> mOverflowStack; // 溢出栈
    unordered_map<string, uint32_t *> *EMMst;         // 更行版本EMMst
public:
    TwochClient()
    {
        cout << "TwochClient, 正常运行!!!" << endl;
    }

    ~TwochClient()
    {
        delete mTwoch;
    }

    /**
     * @brief 使用键值对初始化EMM
     * @param kvList 需要插入的kv列表
     * @param n kv列表的大小
     * @param l 最大容量
     * @return void
     */
    void SetupEMM(vector<KV *> kvList, int n, int l)
    {
        mTwoch = new TwoCH(n);
        mTwoch->setMaxVolume(l);
        // 遍历插入键值对
        for (int i = 0; i < kvList.size(); i++)
        {
            int counter = kvList.at(i)->counter;
            ValueEntry valueE(*kvList.at(i));
            valueE.SpliceRandom();
            valueE.Enc(mKEnc);
            uint32_t fk = getFK(kvList.at(i)->key, strlen(kvList.at(i)->key));
            bool ret = mTwoch->Insert(fk, counter, valueE);
            if (!ret)
            {
                // 插入失败，添加到溢出栈中
                mOverflowStack[kvList.at(i)->key].push_back(*kvList.at(i));
                cout << "Insert To OverflowStack:" << kvList.at(i)->key << "-" << kvList.at(i)->counter << endl;
            }
        }
    }

    /**
     * @brief 查询EMM
     * @param key 需要查询的key
     * @return vector<KV> 查询结果
     * 查询两条路径上的元素以及溢出栈中的元素
     */
    vector<KV> Query(const char *key)
    {
        uint32_t fk = getFK(key, strlen(key));
        vector<ValueEntry> ret = mTwoch->Query(fk);
        vector<KV> queryResult;
        // 解密元素
        for (int i = 0; i < ret.size(); i++)
        {
            if (!ret[i].isEmpty())
            {
                ret[i].Dec(mKEnc);
                ret[i].DivRandom();
                vector<char *> values = ret[i].DivValue();
                vector<KV> kvs = KV::LoadKVList(values);
                for (auto kv : kvs)
                {
                    if (strcmp(kv.key, key) == 0)
                    {
                        queryResult.push_back(kv);
                    }
                }
                // cout << "Query: " << ret[i].p << endl;
            }
            else
            {
                // TODO 空的需要加入空值
                // cout << "Query: NULL" << endl;
            }
        }
        // 查询溢出栈
        auto it = mOverflowStack.find(key);
        if (it != mOverflowStack.end())
        {
            vector<KV> overflowKvs = it->second;
            for (auto kv : overflowKvs)
            {
                if (strcmp(kv.key, key) == 0)
                {
                    queryResult.push_back(kv);
                }
            }
        }
        return queryResult;
    }

    /**
     * @brief 更新元素
     * @param key 需要更新的key
     * @param op 操作符
     * @param kcv 需要更新的kv
     */
    void Update(const char *key, char op, KV kcv)
    {
        // 在st中找不到key,需要初始化
        if (EMMst->find(key) == EMMst->end())
        {
            (*EMMst)[key] = new uint32_t[3]{0, 0};
        }
        uint32_t x = GetXHash(key);
        // Question ！！！！！
        uint32_t y = GetYHash(x, (*EMMst)[key][1]);
        // 获取y
        ValueEntry valueE;
        valueE.SetValue(kcv.key, kcv.counter, kcv.value);
        UpdateEntry updataE(kcv, op);

        // updataE.SpliceRandom();
        // updataE.Enc(mPassword);

        // ++(*EMMst)[key][ST_SUBMIT_TIMES];
        // // 上传服务器
        // // 注意记录counter
        // bemm->AddUpdata(y, updataE);
    }

private:
    /**
     * @brief 计算关键字的哈希值 K = FK(lable)
     */
    uint32_t getFK(const char *lable, int lableLen)
    {
        return XXH32(lable, lableLen, mK);
    }

    /**
     * @brief 计算哈希值x = h(mKu, key)
     */
    uint32_t GetXHash(const char *key)
    {
        return XXH32(key, strlen(key), mKu);
    }

    /**
     * @brief 计算哈希值y = h(x, MMst[st1][1])
     */
    uint32_t GetYHash(uint32_t x, uint32_t st1)
    {
        string x_str = to_string(x);
        return XXH32(x_str.c_str(), x_str.length(), st1);
    }
};

#endif