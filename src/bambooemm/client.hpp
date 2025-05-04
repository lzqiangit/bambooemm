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
        memset(volumeNumArr, 0, sizeof(uint32_t) * mCapacitySize);

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
    void EncryptAndUpload(const char *key, int counter, ValueEntry valueE)
    {
        string hashKey = KV::MakeHashKey(key);
        char *searchKey = KV::MakeSearchKey(hashKey, counter);
        valueE.SpliceRandom();
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
     * 重写更新：更新只用存储op||value即可(不加key可能会碰撞！)
     */
    void Update(const char *key, const Update& update)
    {
        // 在st中找不到key,需要初始化
        if (EMMst->find(key) == EMMst->end())
        {
            (*EMMst)[key] = new uint32_t[3]{0, 0};
        }
        uint32_t x = GetXHash(key);
        // 获取y
        uint32_t y = GetYHash(x, (*EMMst)[key][1]);
        // 获取Update
        UpdateEntry UpdateEntry = update.toUpdateEntry(mPassword);

        ++(*EMMst)[key][ST_SUBMIT_TIMES];
        // 上传服务器
        // 注意记录counter
        bemm->AddUpdata(y, UpdateEntry);
    }

    vector<string> Query(const char *key)
    {
        string hashKey = KV::MakeHashKey(key);

        queryRet = bemm->Query(hashKey);
        
        // 去除随机数, 并解析成value和value_bar
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
        // 解析结果并返回

        return queryValue;
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
     * @brief 将查询结果解析位value和valueBar
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
            if (queryRet[i].len > 0)
            {
                queryValueBar.push_back(ValueEntry());
                queryRet[i].Dec(mPassword);
                queryRet[i].DivRandom();
            
                vector<char*> values = queryRet[i].DivValue();
                vector<KV> kvs = KV::LoadKVList(values);

                // 遍历其中每个元素, 提取其中的目标关键字value, 将无关的值再次存储到queryValueBar中
                for (auto kv : kvs)
                {
                    if (strcmp(kv.key, aimKey) == 0 && !kv.isPadding())
                    {
                        queryValue.push_back(kv.value);
                    }
                    else
                    {
                        queryValueBar.at(i).AppendValue(kv.Splice());
                    }
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
        vector<::Update> updates;
        for (auto ue : ues) {
            updates.push_back(ue.toUpdate(mPassword));
        }
        // 定义辅助变量
        uint32_t curMaxVolume = bemm->getMaxVolume(); // 用于统计更新操作对容量的影响
        uint32_t thisKeyVolume = queryValue.size(); // 用于统计当前key的容量
        int changeVolume = 0; // 用于统计当前key的容量变化

        // 解析更新
        for (int i=0; i<updates.size(); i++)
        {
            
            ::Update &update = updates[i];
            cout << "update: " << update.op << " " << update.value << endl;
            

            switch (update.op)
            {
            case OP_DELETE:
                // 找到目标value并删除即可
                for (int i = 0; i < queryValue.size(); i++)
                {
                    if (strcmp(queryValue[i].c_str(), update.value) == 0)
                    {
                        queryValue.erase(queryValue.begin() + i);
                        break;
                    }
                }
                // 调整该关键字的容量
                changeVolume--;
                break;
            case OP_INSERT:
                // 首先判断容量是否已经超过最大值，如果是，那么上传更新并调整最大容量
                if (thisKeyVolume + changeVolume > curMaxVolume)
                {
                    Update(key, update);
                } else {
                    // 否则，直接将更新插入到queryValue中
                    queryValue.push_back(update.value);
                }
                changeVolume++;
                break;
            default:
                cout << "【ERROR】Undefined Operation!!!";
                break;
            }
        }

        int updateVolume = thisKeyVolume + changeVolume;
        
        // 判断是否超过目前volumeNumArr的极限,是就进行扩容
        if (updateVolume >= mCapacitySize)
        {
            ExpandVNArr();
        }

        volumeNumArr[thisKeyVolume]--;
        volumeNumArr[updateVolume]++;
        // 调整l
        
        if (updateVolume > curMaxVolume)
        {
            // 触发l变大  ** 变大后, counter超过之前l的元素需要提交给update list
            bemm->setMaxVolume(updateVolume);
        } else if (thisKeyVolume == curMaxVolume && volumeNumArr[thisKeyVolume] == 0)
        {
            // l 变小
            for (int i = curMaxVolume; i > 0; i--)
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

    /**
     * 提交更新
     * @param key 目标关键字
     */
    void SubmitUpdate(const char *key)
    {
        // 重新组合queryValue和queryValueBar
        int p = 0;
        vector<ValueEntry> comb;
        for (; p<queryValue.size(); p++) {
            string value = string(key) + "|" + to_string(p) + "|" + string(queryValue[p]);
            queryValueBar[p].AppendValue(value.c_str());
            comb.push_back(queryValueBar[p]);
        }
        for (; p<queryValueBar.size(); p++) {
            comb.push_back(queryValueBar[p]);
        }
        // 添加随机数, 加密并上传更新
        int i = 0;
        for (auto ve : comb)
        {
            EncryptAndUpload(key, i++, ve);
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