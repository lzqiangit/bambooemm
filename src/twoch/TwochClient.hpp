#ifndef TWOCH_CLIENT_H_
#define TWOCH_CLIENT_H_

#include <vector>
#include <xxhash.h>
#include <unordered_map>

#include "KV.hpp"
#include "TwoCH.hpp"
#include "Update.hpp"
#include "UpdateEntry.hpp"
#include "predefine.h"

#define ST_COALESCE_TIMES 0 // 融合次数
#define ST_SUBMIT_TIMES 1   // 提交次数

typedef unsigned int uint32_t;
using std::unordered_map;
using std::vector;
using std::make_pair;

class TwochClient
{
private:
    const uint32_t mK = 123456;  // 计算关键字的哈希值的种子
    const uint32_t mKu = 654321; // 计算
    const char *mKEnc = "135790";

    const char* mPassword = LoadKey();

    TwoCH *mTwoch;                                    // 主存储结构
    unordered_map<string, vector<KV>> mOverflowStack; // 溢出栈
    unordered_map<string, uint32_t *> *EMMst;         // 更行版本EMMst

    /*** 查询和融合相关的变量 ***/
    vector<ValueEntry> queryRet; // 查询结果
    vector<string> queryValue;               // 存储相关的文件索引
    vector<ValueEntry> queryValueBar;       // 存储无关的value
    
    /**
     * 变大 -> 没啥问题
     * 变小 -> 需要考虑到第l号value位的值是否需要挪动到前面来
     * vector中存储bemm中这个key目前是按多少容量存储的, 太大了吧?
     */
    uint32_t *volumeNumArr; // 存储某一容量的key的数量  第0号位置存储数组大小
    int mCapacitySize;

public:
    vector<pair<size_t, double>> mCsvData;
public:
    TwochClient()
    {
        EMMst = new unordered_map<string, uint32_t *>();
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
                // cout << "Insert To OverflowStack:" << kvList.at(i)->key << "-" << kvList.at(i)->counter << endl;
            }
        }
        //TODO 填充？
    }

    /**
     * @brief 查询EMM
     * @param key 需要查询的key
     * @return vector<KV> 查询结果
     * 查询两条路径上的元素以及溢出栈中的元素
     */
    double Query(const char *key)
    {
        uint32_t fk = getFK(key, strlen(key));

        queryRet = mTwoch->Query(fk);
        // 计算返回结果大小
        size_t querySize = 0;
        for (auto qr : queryRet)
        {
            querySize += qr.len;
        }
        
        Timer::getInstance().start();
        ResolveQueryList(key);

        uint32_t cnt = 0;
        // 融合更新
        if (EMMst->find(key) != EMMst->end())
        {
            cnt = (*EMMst)[key][ST_SUBMIT_TIMES];
            if (cnt > 0) {
                CoalesceUpdate(key, cnt);
            }
        }
        Timer::getInstance().stop();
        return Timer::getInstance().getDuration();
        //cout << "融合时间:" << Timer::getInstance().getDuration() << endl;

        
        // return queryValue;
    }

    /**
     * @brief 更新元素
     * @param key 需要更新的key
     * @param op 操作符
     * @param kcv 需要更新的kv
     */
    void UploadUpdate(const char *key, const Update& update)
    {
        // 在st中找不到key,需要初始化
        if (EMMst->find(key) == EMMst->end())
        {
            (*EMMst)[key] = new uint32_t[3]{0, 0};
        }
        uint32_t x = GetXHash(key);
        // 获取y
        uint32_t y = GetYHash(x, (*EMMst)[key][1]);     // TODO
        // 获取Update
        
        ++(*EMMst)[key][ST_SUBMIT_TIMES];
        // 上传服务器
        // 注意记录counter
        vector<UpdateEntry> updateVec;
        UpdateEntry updateEntry = update.toUpdateEntry(mPassword);
        updateVec.push_back(updateEntry);
        ::Update padUpdate;
        for (int i=1; i<mTwoch->getMaxVolume(); i++) {
            updateVec.push_back(padUpdate.toUpdateEntry(mPassword));
        }
        mTwoch->AddUpdata(y, updateVec);
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

/*************************** CV ***************************/
    /**
     * @brief 将查询结果解析位value和valueBar, 并查询溢出栈
     * @param aimKey 目标关键字
     */
    void ResolveQueryList(const char* aimKey)
    {
        // 清楚之前的记录
        queryValue.clear();
        queryValueBar.clear();
        
        // 遍历每一个queryRet, 分别进行:解密，提取其中的目标关键字value
        int queryLen = queryRet.size();
        for (int i = 0; i < queryLen; i++)
        { // 使用引用才能真正实现queryList中元素的解密
            queryValueBar.push_back(ValueEntry());
            if (queryRet[i].len > 0)
            {
                queryRet[i].Dec(mKEnc);
                queryRet[i].DivRandom();
            
                vector<char*> values = queryRet[i].DivValue();
                KV kv = KV::LoadKVList(values).at(0);

                // 遍历其中每个元素, 提取其中的目标关键字value, 将无关的值再次存储到queryValueBar中

                if (strcmp(kv.key, aimKey) == 0 )
                {
                    if (!kv.isPadding()) {
                        queryValue.push_back(kv.value);
                    }
                }
                else
                {
                    queryValueBar.at(i).AppendValue(queryRet.at(i).p);
                }
                
            }
        }
        // 查询溢出栈
        auto it = mOverflowStack.find(aimKey);
        if (it != mOverflowStack.end())
        {
            vector<KV> overflowKvs = it->second;
            for (auto kv : overflowKvs)
            {
                if (strcmp(kv.key, aimKey) == 0)
                {
                    queryValue.push_back(kv.value);
                }
            }
        }
    }

    void CoalesceUpdate(const char *key, uint32_t cnt) {
        // 获取更新并解析
        uint32_t x = GetXHash(key);
        vector<vector<UpdateEntry>> uesV = mTwoch->GetUpdataList(x, cnt);
        // 调整EMMst
        (*EMMst)[key][ST_SUBMIT_TIMES] = 0;
        (*EMMst)[key][ST_COALESCE_TIMES]++;
        // 解析更新
        vector<::Update> updates;
        vector<UpdateEntry> ues = uesV[0]; // 只取第一条更新
        for (auto ue : ues) {
            updates.push_back(ue.toUpdate(mPassword));
        }

        // // 定义辅助变量
        // uint32_t curMaxVolume = mTwoch->getMaxVolume(); // 用于统计更新操作对容量的影响
        // uint32_t thisKeyVolume = queryValue.size(); // 用于统计当前key的容量
        // int changeVolume = 0; // 用于统计当前key的容量变化

        // // 解析更新
        // for (int i=0; i<updates.size(); i++)
        // {
            
        //     ::Update &update = updates[i];
            
            

        //     switch (update.op)
        //     {
        //     case OP_DELETE:
        //         // 找到目标value并删除即可
        //         for (int i = 0; i < queryValue.size(); i++)
        //         {
        //             if (strcmp(queryValue[i].c_str(), update.value) == 0)
        //             {
        //                 queryValue.erase(queryValue.begin() + i);
        //                 break;
        //             }
        //         }
        //         // 调整该关键字的容量
        //         changeVolume--;
        //         break;
        //     case OP_INSERT:
        //         // 首先判断容量是否已经超过最大值，如果是，那么上传更新并调整最大容量
        //         if (thisKeyVolume + changeVolume > curMaxVolume)
        //         {
        //             UploadUpdate(key, update);
        //         } else {
        //             // 否则，直接将更新插入到queryValue中
        //             queryValue.push_back(update.value);
        //         }
        //         changeVolume++;
        //         break;
        //     default:
        //         cout << "【ERROR】Undefined Operation!!!";
        //         break;
        //     }
        // }

        // int updateVolume = thisKeyVolume + changeVolume;
        
        // // 判断是否超过目前volumeNumArr的极限,是就进行扩容
        // if (updateVolume >= mCapacitySize)
        // {
        //     ExpandVNArr();
        // }

        // volumeNumArr[thisKeyVolume]--;
        // volumeNumArr[updateVolume]++;
        // // 调整l
        
        // if (updateVolume > curMaxVolume)
        // {
        //     // 触发l变大  ** 变大后, counter超过之前l的元素需要提交给update list
        //     mTwoch->setMaxVolume(updateVolume);
        // } else if (thisKeyVolume == curMaxVolume && volumeNumArr[thisKeyVolume] == 0)
        // {
        //     // l变小
        //     for (int i = curMaxVolume; i > 0; i--)
        //     {
        //         if (volumeNumArr[i] != 0)
        //         {
        //             mTwoch->setMaxVolume(i);
        //             break;
        //         }
        //     }
        // }
        // /**************************************** 判断并收缩EMM中元素至符合当前容量 ****************************************/
        // // 调整EMMst
        // SubmitUpdate(key);
    }

    /**
     * 提交更新
     * @param key 目标关键字
     */
    void SubmitUpdate(const char *key)
    {
        // 重新组合queryValue和queryValueBar
        int p = 0;
        vector<ValueEntry> comb;
        int treeHeight = mTwoch->getTreeHeight();
        int maxVol = mTwoch->getMaxVolume();
        // 清空溢出栈中目标关键字的元素
        auto it = mOverflowStack.find(key);
        if (it != mOverflowStack.end())
        {
            it->second.clear();
        }
        // 遍历每一个索引
        int size = treeHeight * 2;
        for (int i=0; i<queryValueBar.size(); i++)
        {

            int counter = i % size;   // counter

            bool unsaved = true;
            KV kv(key, counter, queryValue[i].c_str());
            // 遍历几个可能的位置，如果有空位就添加，如果没有就加入到溢出栈
            for (int p=size*counter; p<size*counter +size; p++) {
                if (queryValueBar[i].isEmpty()) {
                    queryValueBar[i].AppendValue(kv.Splice());
                    unsaved = false;
                    break;
                }
            }
            // 如果没有空位就添加到溢出栈中
            if (unsaved) {
                mOverflowStack[key].push_back(kv);
            }
        }
        // 判断queryValue是否超过最大容量, 超出部分直接添加到溢出栈

        // 
        

        
    }

        /**
     * 将value添加随机数,并更新服务器中key对应位置的值,
     * 为了防止新添加新key可能导致的出现不存在的指纹的问题
     */
    void EncryptAndUpload(const char *key, int counter, ValueEntry valueE)
    {
        // string hashKey = KV::MakeHashKey(key);
        // char *searchKey = KV::MakeSearchKey(hashKey, counter);
        // valueE.SpliceRandom();
        // valueE.Enc(mPassword);
        

        // if (mTwoch->isExistKeyCounter(hashKey, counter))
        // {
        //     mTwoch->ReInsert(searchKey, valueE);
        // }
        // else
        // {
        //     mTwoch->Insert(hashKey, counter, valueE);
        // }
    }

    void ExpandVNArr()
    {
        int oldLen = mCapacitySize;
        mCapacitySize *= 1.5;

        uint32_t *newVNA = new uint32_t[mCapacitySize];
        // 初始化
        memset(newVNA, 0, sizeof(uint32_t) * mCapacitySize);
        // 拷贝
        memcpy(newVNA, volumeNumArr, sizeof(uint32_t) * oldLen);
        // 删除
        delete[] volumeNumArr;
        volumeNumArr = newVNA;
    }

public:
    size_t getMemOverHead()
    {
        size_t size = 0;
        cout << "==================================================================" << endl;
        size += this->mTwoch->getMemOverhead();
        cout << "-----------------------------------------------------------------" << endl;
        // 计算溢出栈mOverflowStack中所有KV的总大小
        size_t overflowSize = 0;
        for (auto &pair : mOverflowStack)
        {
            for (auto &kv : pair.second)
            {
                overflowSize += kv.getMemOverhead();
            }
        }

        cout << "溢出栈总空间:" << getMemSizeStr(overflowSize) << endl;

        cout << "==================================================================" << endl;
        return size;
    }
private:
    const char* concatInt(int a, int b) {
        string str = to_string(a) + "|" + to_string(b);
        return copy_const_str(str.c_str());
    }
};

#endif