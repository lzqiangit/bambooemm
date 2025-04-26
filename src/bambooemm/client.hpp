#ifndef CLIENT_H_
#define CLIENT_H_

#include "bambooemm.hpp"
#include "utils.hpp"
#include "predefine.h"
#include "filterposition.hpp"
#include "Timer.hpp"
#include "Update.hpp"

#include <unordered_map>
#include <string>
#include <sstream>

#include "xxhash.h"

#define ST_COALESCE_TIMES 0 // 融合次数
#define ST_SUBMIT_TIMES 1   // 提交次数
// #define ST_MAX_VOLUME 2         // ？

using std::pair;

class Client
{
private:
    /* data */
    int n;
    BambooEMM *bemm;
    const uint32_t mKe = 123456;      // 用于加密EMM中数据的密钥
    const uint32_t mKu = 654321;     // 用于加密更新数据的密钥
    const uint32_t mSu = 135790;     // 用于更新计算HashX的种子
    unordered_map<string, uint32_t *> *EMMst; // 分别存储
    const char* mPassword = LoadKey();
    /************* 用于查询的数据结构 **************/
    vector<ValueEntry> queryList;           // 存储服务器返回的结果
    vector<vector<KV>> resolvedQueryList;   // 存储解密, 去随机数, 分割后的结果
    int *queryValueMap;  // 存储对应counter的value的位置
    int queryValueMapSize;
    vector<int> preRandom;   // 存储解密后的随机数
    int realVolume;
    vector<KV> queryRet;    // 存储查询结果
    
    /**
     * 变大 -> 没啥问题
     * 变小 -> 需要考虑到第l号value位的值是否需要挪动到前面来
     * vector中存储bemm中这个key目前是按多少容量存储的, 太大了吧?
     */
    uint32_t *volumeNumArr; // 存储某一容量的key的数量  第0号位置存储数组大小
    int mCapacitySize;

public:
    Client(/* args */)
    {
        EMMst = new unordered_map<string, uint32_t *>();
    }

    ~Client()
    {
    }

    /**
     * 扩容volumeNumArr和queryValueMap
     */
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

        // queryValueMap
        delete[] queryValueMap;
        queryValueMap = new int[mCapacitySize];
        memset(queryValueMap, 0, sizeof(int) * mCapacitySize);
    }

    /**
     * 传入初始数据的EMM，以及kv总数和最大最大容量l
     */
    void SetupEMM(vector<KV *> kvList, int n, int l)
    {
        this->n = n;
        vector<KV *> maxCounterKVList; // 存储每个key中counter最大的元素
        this->bemm = new BambooEMM();
        this->bemm->Setup(2, MIN_STAR_CAP, l); // LoadKey应该作为函数参数传入好一些!!!
        char *tempKey = kvList.at(0)->key;
        // 初始化volumeNum 和 queryValueMap
        mCapacitySize = l * 1.5;
        volumeNumArr = new uint32_t[mCapacitySize];
        queryValueMap = new int[mCapacitySize];
        memset(volumeNumArr, 0, sizeof(uint32_t) * mCapacitySize);
        memset(queryValueMap, 0, sizeof(int) * mCapacitySize);

        // 插入初始元素
        for (int i = 0; i < kvList.size(); i++) // 考虑直接通过数据库来求得每个key的容量
        {
            this->bemm->SetupInsert(kvList.at(i), mKe); // counter必须从0开始而且连续 yes
            if (i == kvList.size() - 1)
            {
                maxCounterKVList.push_back(kvList.at(i));
                ++volumeNumArr[kvList.at(i)->counter + 1];
                continue;
            }
            if (strcmp(tempKey, kvList.at(i + 1)->key) != 0)
            {
                maxCounterKVList.push_back(kvList.at(i));
                ++volumeNumArr[kvList.at(i)->counter + 1];
                tempKey = kvList.at(i + 1)->key;
            }
        }
        PaddingStep(maxCounterKVList, l);
        // 加密
        bemm->AddRandomAndEncrypt(mPassword);
        // 释放maxCounterKVList
    }

    /**
     * 考虑到先插入后填充,或许可以另开一个填充函数
     */
    void PaddingStep(vector<KV *> kvList, int l)
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
                KV *kv = new KV(key, ++counter);
                if (!(this->bemm->isExistKeyCounter(KV::MakeHashKey(key), kv->counter)))
                {
                    this->bemm->SetupInsert(kv, mKe);
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

    BambooEMM *getBEMM()
    {
        return this->bemm;
    }

    /**
     * 查询获得了 : key || counter || value || random ——> hash(hash(key)||counter) -> new splic(value)
     * 客户端重新解析生成 KV，装入valueEntry
     * 在做了必要的修改之后，将修改后的kvList传入此函数
     * 此函数会拼接 splic_key = hash(hash(key)||counter) 和 splic_val = splic(value),这里splic_val的random值会修改      // ? 在kv中存储random y
     * 然后通过splic_key的值作为key去组合value，传递给服务端bambooemm进行UpdateValue    // char* 转 string作为key，否则无法正常比较值，而是比较char*的指针地址
     */

    /**
     * 将value添加随机数,并更新服务器中key对应位置的值,
     * 为了防止新添加新key可能导致的出现不存在的指纹的问题
     */
    void EncryptAndUpload(const char *key, int counter, ValueEntry valueE, int preRandom)
    {
        string hashKey = KV::MakeHashKey(key);
        char *searchKey = KV::MakeSearchKey(hashKey, counter);
        valueE.SpliceRandom(preRandom);
        valueE.Enc(mPassword);

        if (bemm->isExistKeyCounter(hashKey, counter))
        {
            bemm->ReInsert(searchKey, valueE);
        }
        else
        {
            bemm->Insert(hashKey, counter, valueE);
        }
    }

    /**
     * 更新EMMst中的数据,调用服务端添的接口,向EMMu中更新 key||counter位置的值
     * x <- hash(key||MMst[key][0], len, Ku);   // 用于计算EMMu中index的 中间值
     * y <- hash(MMst[kye][1]||x, len, Ku) % EMMu.length();     // EMMu的索引值
     * z <- Enc(Kenc, (op, counter,v));
     * EMMu[y] <- z
     * EMMst[label][1]++;
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

        updataE.SpliceRandom();
        updataE.Enc(mPassword);

        ++(*EMMst)[key][ST_SUBMIT_TIMES];
        // 上传服务器
        // 注意记录counter
        bemm->AddUpdata(y, updataE);
    }

    vector<KV> Query(const char *key)
    {
        queryRet.clear();
        // cout << "开始调用的时间:" << Timer::getInstance().getDuration() << "ms" << endl;
        string hashKey = KV::MakeHashKey(key);
        // cout << "生成HashKey的时间:" << Timer::getInstance().getDuration() << "ms" << endl;
        queryList.clear();

        //Timer::getInstance().start();
        queryList = bemm->Query(hashKey);
        //Timer::getInstance().stop();
        
        // 去除随机数, 并解析
        ResolveQueryList();
        MakeQueryMap(key);
        uint32_t cnt = 0;
        if (EMMst->find(key) != EMMst->end())
        {
            cnt = (*EMMst)[key][ST_SUBMIT_TIMES];
            if (cnt > 0) {
                CoalesceUpdate(key, cnt);
            }
        }
        // 解析结果并返回

        for (auto kvs : resolvedQueryList)
        {
            for (auto kv : kvs)
            {
                if (strcmp(kv.key, key) == 0 && (!kv.isPadding()))
                {
                    queryRet.push_back(kv);
                }
            }
        }

        return queryRet;
    }

    /**********************************************  Splice  *******************************************************************/
    uint32_t GetXHash(const char *key)
    {   
        return XXH32(key, strlen(key), mSu);
    }

    char *SpliceOpVal(char op, uint32_t counter, char *val)
    {
        stringstream ss;

        ss << op << "|" << counter << "|" << val;
        string splice = ss.str();
        char *ret = new char[splice.length() + 1];
        memset(ret, 0, splice.length() + 1);
        memcpy(ret, splice.c_str(), splice.length());
        return ret;
    }
    /**
     * 去除解密queryList, 去除尾随机数, 并解析为resolvedQueryList
     */
    void ResolveQueryList()
    {
        resolvedQueryList.clear();
        // 清空preRandom, 注意释放内存
        preRandom.clear();
        
        int queryLen = queryList.size();
        for (int i = 0; i < queryLen; i++)
        { // 使用引用才能真正实现queryList中元素的解密
            if (queryList[i].getLen() > 0)
            {
                queryList[i].Dec(mPassword);    // 解密
                preRandom.push_back(queryList[i].DivRandom()); // 去除随机数, 《需要记录下来, 然后防止生成的随机数同上次相同》
                // 解析value值
                vector<char*> values = queryList[i].DivValue();
                vector<KV> kvs = KV::LoadKVList(values);
                resolvedQueryList.push_back(kvs);
            } else {
                // ? 什么情况下会出现长度为0的valueE, 插入新的value ???
            }
        }
    }

    /**
     * 生成映射, 统计非填充有意义的值的数量
     */
    void MakeQueryMap(const char *key) {
        // 初始化queryValueMap
        memset(queryValueMap, 0, sizeof(int) * mCapacitySize);

        realVolume = 0;
        int queryLen = resolvedQueryList.size();
        for (int i = 0; i < queryLen; i++)
        { // 使用引用才能真正实现queryList中元素的解密
            for (int j = 0; j < resolvedQueryList[i].size(); j++) {
                if (strcmp(key, resolvedQueryList[i][j].key) == 0) {
                    queryValueMap[i] = j;
                    if (!resolvedQueryList[i][j].isPadding()) {
                        ++realVolume;
                    }
                    break;
                }
            }
        }
    }

    void CoalesceUpdate(const char *key, uint32_t cnt) {
        // 获取更新并解析
        uint32_t x = GetXHash(key);
        vector<UpdateEntry> ues = bemm->GetUpdataList(x, cnt);
        // 调整EMMst
        (*EMMst)[key][ST_SUBMIT_TIMES] = 0;
        (*EMMst)[key][ST_COALESCE_TIMES]++;
        // 解析更新
        vector<::Update> updates = Update::ResolveFromUpdateEntries(ues, mPassword);
        // 定义辅助变量
        int updateVolume = realVolume; // 用于统计更新操作对容量的影响
        // 服务端最大容量
        uint32_t currMaxVolume = bemm->getMaxVolume();
        
        for (int i=0; i<updates.size(); i++)
        {
            
            ::Update update = updates[i];

            // 输出update的信息
            cout << "Update:" << update.op << "|" << update.key << "|" << update.counter << "|" << update.value << endl;

            switch (update.op)
            {
            case OP_DELETE:
                // 循环覆盖
                for (int i=update.counter; i<updateVolume-1; i++) {
                    resolvedQueryList[i][queryValueMap[i]].value =  resolvedQueryList[i+1][queryValueMap[i+1]].value;
                }   
                // 最后一个元素转化为填充值
                resolvedQueryList[updateVolume-1][queryValueMap[updateVolume-1]].BePadding();   
                // 更新未完成的update的counter
                for (int j=i+1; j<updates.size(); j++) {
                    if (updates[j].counter > update.counter) {
                        --updates[j].counter;
                    }
                }
                --updateVolume;
                break;
            case OP_EDIT: // EDIT要求这个值之前必须要已经存在的
                resolvedQueryList[update.counter][queryValueMap[update.counter]].setValue(update.value);
                break;
            case OP_INSERT:
                // 如果对应counter有填充, 那么就直接修改对应值就可以了
                // 判断当前容量是否超标, 是上传插入到服务器, 等待下一次搜索,更新updateVolume后直接退出即可
                if (updateVolume >= currMaxVolume) {
                    // 上传更新！！！！！！！！！！！！！EMMST需要调整
                    KV kv(update.key, update.counter, update.value);
                    Update(update.key, update.op, kv);
                    queryRet.push_back(kv);
                } else {
                    resolvedQueryList[updateVolume][queryValueMap[updateVolume]].setValue(update.value);
                }
                ++updateVolume;
                break;
            default:
                cout << "【ERROR】Undefined Operation!!!";
                break;
            }
        }

        
        // 判断是否超过目前volumeNumArr的极限,是就进行扩容
        if (updateVolume >= mCapacitySize)
        {
            ExpandVNArr();
        }

        volumeNumArr[realVolume]--;
        volumeNumArr[updateVolume]++;
        // 调整l
        
        if (updateVolume > currMaxVolume)
        {
            // 触发l变大  ** 变大后, counter超过之前l的元素需要提交给update list
            bemm->setMaxVolume(updateVolume);
        } else if (realVolume == currMaxVolume && volumeNumArr[realVolume] == 0)
        {
            // l 变小
            for (int i = currMaxVolume; i > 0; i--)
            {
                if (volumeNumArr[i] != 0)
                {
                    bemm->setMaxVolume(i);
                    break;
                }
            }
        }
        /**************************************** 判断并收缩EMM中元素至符合当前容量 ****************************************/
        // 调整EMMst
        SubmitUpdate(key);
    }

    void SubmitUpdate(const char *key)
    {
        for (int i = 0; i < resolvedQueryList.size(); i++)
        {
            ValueEntry ve(resolvedQueryList[i]);
            EncryptAndUpload(key, i, ve, preRandom[i]);
        }
    }

    size_t getMemOverHead()
    {
        size_t size = 0;
        cout << "==================================================================" << endl;
        cout << "--------------------------------Client----------------------------" << endl;

        for (const auto &pair : (*EMMst))
        {
            size += pair.first.length();
            size += sizeof(uint32_t) * 3;
        }
        cout << "EMMst:" << size << endl;
        // volumeNumArr大小
        size_t volumeNumArrSize = sizeof(uint32_t) * volumeNumArr[0];
        cout << "volumeNumArr:" << volumeNumArrSize << endl;
        size += volumeNumArrSize;

        cout << "others" << sizeof(Client) << endl;
        size += sizeof(Client);

        cout << "sum" << size << endl;
        // 调用函数统计服务端
        size += this->bemm->getMemOverhead();
        cout << "-----------------------------------------------------------------" << endl;
        cout << "CS总占用空间:" << getMemSizeStr(size) << endl;
        cout << "==================================================================" << endl;
        return size;
    }
};
#endif