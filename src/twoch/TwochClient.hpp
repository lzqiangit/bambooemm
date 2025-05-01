#ifndef TWOCH_CLIENT_H_
#define TWOCH_CLIENT_H_

#include <vector>
#include <xxhash.h>
#include <unordered_map>

#include "KV.hpp"
#include "TwoCH.hpp"

typedef unsigned int uint32_t;
using std::vector;
using std::unordered_map;

class TwochClient
{
private:
    const uint32_t mK = 123456;     // 计算关键字的哈希值的种子
    const uint32_t mKu = 654321;
    const char* mKEnc = "135790";

    TwoCH *mTwoch;          // 主存储结构
    unordered_map<string, vector<KV>> mOverflowStack;   // 分别存储
public:
    TwochClient() {
        cout << "TwochClient, 正常运行!!!" << endl;
    }
    
    ~TwochClient() {
        delete mTwoch;
    }

    /**
     * @brief 使用键值对初始化EMM
     * @param kvList 需要插入的kv列表
     * @param n kv列表的大小
     * @param l 最大容量
     * @return void
     */
    void SetupEMM(vector<KV *> kvList, int n, int l) {
        mTwoch = new TwoCH(n);
        mTwoch->setMaxVolume(l);
        // 遍历插入键值对 
        for (int i = 0; i < kvList.size(); i++) {
            int counter = kvList.at(i)->counter;
            ValueEntry valueE(*kvList.at(i));
            valueE.SpliceRandom();
            valueE.Enc(mKEnc);
            uint32_t fk = getFK(kvList.at(i)->key, strlen(kvList.at(i)->key));
            bool ret = mTwoch->Insert(fk, counter, valueE);
            if (!ret) {
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
    vector<KV> Query(const char *key) {
        uint32_t fk = getFK(key, strlen(key));
        vector<ValueEntry> ret = mTwoch->Query(fk);
        vector<KV> queryResult;
        // 解密元素
        for (int i = 0; i < ret.size(); i++) {
            if (!ret[i].isEmpty()) {
                ret[i].Dec(mKEnc);
                ret[i].DivRandom();
                vector<char*> values = ret[i].DivValue();
                vector<KV> kvs = KV::LoadKVList(values);
                for (auto kv : kvs) {
                    if (strcmp(kv.key, key) == 0) {
                        queryResult.push_back(kv);
                    }
                }
                //cout << "Query: " << ret[i].p << endl;
            } else {
                //TODO 空的需要加入空值 
                //cout << "Query: NULL" << endl;
            }
            
        }
        // 查询溢出栈
        auto it = mOverflowStack.find(key);
        if (it != mOverflowStack.end()) {
            vector<KV> overflowKvs = it->second;
            for (auto kv : overflowKvs) {
                if (strcmp(kv.key, key) == 0) {
                    queryResult.push_back(kv);
                }
            }
        }
        return queryResult;
    }   

    

private:
    /**
     * @brief 计算关键字的哈希值 K = FK(lable)
     */
    uint32_t getFK(const char* lable, int lableLen) {
        return XXH32(lable, lableLen, mK);
    }


};

#endif