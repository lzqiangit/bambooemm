#ifndef CLIENT_H_
#define CLIENT_H_

#include "bambooemm.hpp"
#include "utils.hpp"
#include "predefine.h"
#include "filterposition.hpp"

#include <unordered_map>
#include <string>
#include <sstream>


#define ST_COALESCE_TIMES 0
#define ST_SUBMIT_TIMES 1
#define ST_MAX_VOLUME 2   
using std::pair;

class Client
{
private:
    /* data */
    int n;
    BambooEMM *bemm;
    uint32_t K, Ku;   // 种子
    unordered_map<string, uint32_t*>  *EMMst;       // 分别存储
    /**
     * 变大 -> 没啥问题
     * 变小 -> 需要考虑到第l号value位的值是否需要挪动到前面来 
     * vector中存储bemm中这个key目前是按多少容量存储的, 太大了吧?
     */
    uint32_t *volumeNumArr;    // 存储某一容量的key的数量  第0号位置存储数组大小

    uint32_t getVolumeNumArrLength();
    void setVolumeNumArrLength(uint32_t length);
    void ExpandVNArr();     // 扩展
public:
    Client(/* args */);
    ~Client();

    void SetupEMM(vector<KV *> kvList, int n, int l);
    void PaddingStep(vector<KV *> kvList, int l);
    BambooEMM *getBEMM();
    /**
     * 此函数用于更新操作,再融合完成value完成后,给value添加随机数,加密后上传服务器更新valueEntry
     * key 需要重写的key
     * valueE 需要重写的未添加随机数且未加密状态的valueEntry
     * preRandom 该value的前一个随机数 
     */
    void EncryptAndUpload(const char *key, int counter, ValueEntry valueE, int preRandom);

    /**
     * 更新函数: 
     * key: 需要更新的key <- hash(key)||counter
     * counter
     * op: 需要对 key[counter]执行的操作
     * value: 操作后的值 insert,edit需要value而delete不需要
     */
    void Update(char *key, char op, KV kcv);
    
    /**
     * 查询
     */
    vector<KV> Query(const char *key);

private:
    uint32_t GetXHash(const char *key);
    /**
     * 拼接 操作对应的操作符和val
     */
    char *SpliceOpVal(char op, uint32_t counter, char *val); 
    // 融合
    vector< vector<KV> > Coalesce(const char *key, int cnt, vector<ValueEntry> old);    
    bool ShrinkVolume(vector< vector<KV> > &resolueQuery, const char *key, int preVolume);
    void SubmitUpdate(vector<vector<KV>> resolueQuery, const char *key, int *preRandom);
};

Client::Client(/* args */)
{
    EMMst = new unordered_map<string, uint32_t*>();
    K = 3;
    Ku = 3;
}

Client::~Client()
{
}

uint32_t Client::getVolumeNumArrLength() {
    return volumeNumArr[0];
}

void Client::setVolumeNumArrLength(uint32_t length) {
    volumeNumArr[0] = length;
}

void Client::ExpandVNArr() {
    // 
    int oldLen = getVolumeNumArrLength();
    int newLen = oldLen * 1.5;
    uint32_t *newVNA = new uint32_t[newLen];
    // 初始化
    memset(newVNA, 0, sizeof(uint32_t) * newLen);
    // 拷贝
    memcpy(newVNA, volumeNumArr, sizeof(uint32_t) * oldLen);
    // 删除
    delete []volumeNumArr;
    volumeNumArr = newVNA;
    // 重新设置容量大小
    setVolumeNumArrLength(newLen);
}

/**
 * 传入初始数据的EMM，以及kv总数和最大最大容量l
 */
void Client::SetupEMM(vector<KV *> kvList, int n, int l)
{
    this->n = n;
    vector<KV *> maxCounterKVList; // 存储每个key中counter最大的元素
    this->bemm = new BambooEMM();
    this->bemm->Setup(2, MIN_STAR_CAP, l, LoadKey());           // LoadKey应该作为函数参数传入好一些!!!
    char *tempKey = kvList.at(0)->key;
     // 初始化volumeNum
    int volumeNumSize = l * 1.5;
    volumeNumArr = new uint32_t[volumeNumSize];
    for (int i=0; i<volumeNumSize; i++) {
        volumeNumArr[i] = 0;
    }
    setVolumeNumArrLength(volumeNumSize);
    for (int i = 0; i < kvList.size(); i++)
    {
        this->bemm->SetupInsert(kvList.at(i)); // counter必须从0开始而且连续 yes
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
    bemm->AddRandomAndEncrypt(LoadKey());
}

/**
 * 考虑到先插入后填充,或许可以另开一个填充函数
 */
void Client::PaddingStep(vector<KV *> kvList, int l)
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
                this->bemm->SetupInsert(kv);
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
 * 客户端重新解析生成 KV，装入valueEntry
 * 在做了必要的修改之后，将修改后的kvList传入此函数
 * 此函数会拼接 splic_key = hash(hash(key)||counter) 和 splic_val = splic(value),这里splic_val的random值会修改      // ? 在kv中存储random y
 * 然后通过splic_key的值作为key去组合value，传递给服务端bambooemm进行UpdateValue    // char* 转 string作为key，否则无法正常比较值，而是比较char*的指针地址
 */


/**
 * 将value添加随机数,并更新服务器中key对应位置的值,
 * 为了防止新添加新key可能导致的出现不存在的指纹的问题
 */
void Client::EncryptAndUpload(const char *key, int counter, ValueEntry valueE, int preRandom)
{
    char *searchKey = KV::MakeSearchKey(KV::MakeHashKey(key), counter);
    valueE.SpliceRandom(preRandom);
    valueE.Enc(LoadKey());
    string hashKey = KV::MakeHashKey(key);
    if (bemm->isExistKeyCounter(hashKey, counter)) {
        bemm->ReInsert(searchKey, valueE);
    } else {
        bemm->Insert(KV::MakeHashKey(key), counter, valueE);
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
void Client::Update(char *key, char op, KV kcv) {
    // 在st中找不到key,需要初始化
    if (EMMst->find(key) == EMMst->end()) {     
        (*EMMst)[key] = new uint32_t[3]{0, 0, bemm->getMaxVolume()};
    }
    uint32_t x = GetXHash(key);
    // Question ！！！！！
    uint32_t y = GetYHash(x, (*EMMst)[key][1]);
    // 获取y
    ValueEntry valueE;
    valueE.SetValue(kcv.key, kcv.counter, kcv.value);
    UpdataEntry updataE(valueE, op);
    
    updataE.SpliceRandom();
    updataE.Enc(LoadKey());


    ++(*EMMst)[key][ST_SUBMIT_TIMES];
    // 上传服务器
    // 注意记录counter
    bemm->AddUpdata(y, updataE);
}

vector<KV> Client::Query(const char *key) {

    string hashKey = KV::MakeHashKey(key);
    vector<ValueEntry> queryList;
    if (EMMst->find(key) != EMMst->end() && (*EMMst)[key][ST_MAX_VOLUME] > bemm->getMaxVolume()) {
        queryList = bemm->Query(hashKey, (*EMMst)[key][ST_MAX_VOLUME]);
    } else {
        queryList = bemm->Query(hashKey);
    }
    vector< vector<KV> > resolueQuery;

    // 

    uint32_t cnt = 0;
    if ( EMMst->find(key) != EMMst->end() ) {
        cnt = (*EMMst)[key][ST_SUBMIT_TIMES];
    }
    // 融合更新, 并解析结果
    resolueQuery = Coalesce(key, cnt, queryList);
    // } else {
    //     for (int i=0; i<queryList.size(); i++) {
    //         ValueEntry ve = queryList.at(i);
    //         ve.DivRandom();
    //         // 解析value值
    //         vector<char*> values = ve.DivValue();
    //         vector<KV> kvs = KV::LoadKVList(values);
    //         resolueQuery.push_back(kvs);
    //     }
    // }
    // 找到真正有意义的解
    vector<KV> ret;
    for (auto kvs : resolueQuery) {
        for (auto kv : kvs) {
            if (strcmp(kv.key, key) == 0 && (!kv.isPadding())) {
                ret.push_back(kv);
            }
        }
    }
    // 返回正真的元素
    return ret;
}

/**********************************************  Splice  *******************************************************************/
uint32_t Client::GetXHash(const char *key) {
    string keyStr = key;
    string st0Str = to_string( (*EMMst)[key][ST_COALESCE_TIMES] );
    string xStr = keyStr + "|" + st0Str; 
    char *ret = new char[xStr.length() + 1];
    memset(ret, 0, xStr.length() + 1);
    memcpy(ret, (char*)xStr.c_str(), xStr.length());
    return BOBHash::run(ret, strlen(ret), 3);
}

char *Client::SpliceOpVal(char op, uint32_t counter, char *val) {
    stringstream ss;

    ss << op << "|" << counter << "|" << val;
    string splice = ss.str();
    char *ret = new char[splice.length() + 1];
    memset(ret, 0, splice.length() + 1);
    memcpy(ret, splice.c_str(), splice.length());
    return ret;
}

vector< vector<KV> > Client::Coalesce(const char *key, int cnt, vector<ValueEntry> queryVEL) {


    int queryLen = queryVEL.size();
    int *preRandom = new int[queryLen];
    int notEmptyNum = 0, preNotEmptyNum;     // 非空元素个数, 当前key的实际容量
    vector<int> deledIndexs;     // 用于记录删除的元素索引,用于支持一次融合多次更新
    bool isAdjust = false;

    unordered_map<int, pair<int, int> > map;    // counter -> (ValueEntry_index, value_index)
    vector< vector<KV> > resolueQuery;      // 用于存储解析的查询结果, 并在其上进行更新操作, 
    // 解密并 解析查询结果, 找到每个 key||counter 对应的 qeryVEL 的索引, 并记录下随机数
    for (int i=0; i<queryLen; i++) {   // 使用引用才能真正实现queryList中元素的解密
        if (queryVEL[i].getLen() > 0) {
            queryVEL[i].Dec(LoadKey());
            preRandom[i] = queryVEL[i].DivRandom();    // 记录随机数
            // 解析value值
            vector<char*> values = queryVEL[i].DivValue();
            vector<KV> kvs = KV::LoadKVList(values);
            resolueQuery.push_back(kvs);
            // 遍历判断
            for (int j=0; j<kvs.size(); j++) {
                KV kv = kvs.at(j);
                if ( strcmp( kv.key, key) == 0) {
                           // 还可能有找不到的counter,这种counter就是共用了其他value的值！
                    if (!kv.isPadding()) {
                        map[kv.counter] = {i, j};
                        notEmptyNum++;
                // 是填充值
                // - 在扩展l中, 可能出现两个同一counter的值, 填充值一定在末尾, 如果下前面找到了相同的counter值,一定要将前面的值移动到此处 
                // 没有找到,正常构建索引即可
                    } else if (map.find(i) != map.end()) {
                        // 此种情况只会出现在 导致l 增大的key 在其导致l增大后的第二次查询中
                        // 如果是填充值, 还需要判断
                        // 找到了, 直接移动即可
                        pair<int, int> prePos = map[i];    // 对应值存储的位置
                        // 复制resolueQuery中对应位置的值到当前的i的位置
                        KV tempKV = resolueQuery[prePos.first][prePos.second];
                        // 删除原位置的值
                        resolueQuery[prePos.first].erase( resolueQuery[prePos.first].begin() + prePos.second );
                        // 修改索引
                        map[i] = {i, 0};
                        resolueQuery[i].erase(resolueQuery[i].begin() + j);
                        resolueQuery[i].push_back(tempKV);
                        isAdjust = true;
                    } else {
                        // 填充值
                        map[kv.counter] = {i, j};
                    }
                }
            }
        } else {
            // 此种情况只会出现在 出现新的key 的情况下
            // 判断是否存在长度为0的valueE,如果有,那么就说明这个key是一个新的key,需要将这些valueE中填充值
            // 如果这个key是一个新值,但是其搜索出来的valueE中均有值,那么就不用为这个新key进行特殊的处理
            // 如果是因为l增大导致的空值,那么就直接对于counter值到这个空值位即可
            KV newPaddingKV((char*)key, i);
            resolueQuery[i].push_back(newPaddingKV);
            map[i] = {i, 0};
        }      
    }    

    preNotEmptyNum = notEmptyNum;   // preNotEmptyNum 记录在融合操作之前的实际容量
    if (cnt <= 0) {
        if (  (*EMMst).find(key) != (*EMMst).end() && ShrinkVolume(resolueQuery, key, (*EMMst)[key][ST_MAX_VOLUME]) ) {
            SubmitUpdate(resolueQuery, key, preRandom);
            (*EMMst)[key][ST_MAX_VOLUME] = bemm->getMaxVolume();
        } else {
            if (isAdjust)   SubmitUpdate(resolueQuery, key, preRandom);
        }
        return resolueQuery;
    }
    // 解析更新
    uint32_t x = GetXHash(key);
    vector<UpdataEntry> ues = bemm->GetUpdataList(x, cnt);
    for (UpdataEntry &updata : ues) {
        updata.Dec(LoadKey());
        updata.DivRandom();
        char *value = updata.DivValue();
        KV updataKv(value);                         // 更新后的值
        char op = updata.DivOP();

        int upCounter = updataKv.counter;           // 操作的counter
        int preDeledNum = 0;
        // 调整因删除产生的偏移
        for (int deled : deledIndexs) {
            if (deled <upCounter) {
                preDeledNum++;
            }
        }
        upCounter -= preDeledNum;

        pair<int, int> prePos, aftPos, finalPos;
        int minIndex;
        switch (op)
        {
        case OP_DELETE:
            
            // 类似顺序表, 依次将前面元素的counter减一,并向前挪动一位
            for (int i=upCounter; i<notEmptyNum-1; i++) {
                // 找到元素
                prePos = map[i];
                aftPos = map[i + 1];
                // 移动位置
                resolueQuery[prePos.first][prePos.second] = resolueQuery[aftPos.first][aftPos.second];
                // 修改counter
                resolueQuery[prePos.first][prePos.second].counter--;
            }
            finalPos = map[notEmptyNum-1];       
            resolueQuery[finalPos.first][finalPos.second].BePadding();  // 将最后一个非填充值修改为填充值
            notEmptyNum--;      // 需要融合多个操作的时候, 需要考虑非空值的变化
            deledIndexs.push_back(upCounter + preDeledNum);       // 实际的push值需要是融合前的
            // 其他的填充值不用变动! 结束!
            break;
        case OP_EDIT:       // EDIT要求这个值之前必须要已经存在的
            // 修改value即可
            if (map.find(upCounter) == map.end()) {
                // 没有找到, 直接返回修改失败
                cout << "对不存在的counter值无法进行修改操作!" << endl;
            }
            prePos = map[upCounter];
            resolueQuery[prePos.first][prePos.second].setValue(updataKv.value);
            break;
        case OP_INSERT:
            // 如果对应counter有填充, 那么就直接修改对应值就可以了
            if ( map.find(notEmptyNum) != map.end() ) {
                prePos = map[notEmptyNum];
                resolueQuery[prePos.first][prePos.second].setValue(updataKv.value);
            } else {        // 如果没有, 就填充到容量最小的位置
                // 找到最小容量的位置
                minIndex = 0;
                for (int i=1; i<resolueQuery.size(); i++) {
                    if (resolueQuery[minIndex].size() > resolueQuery[i].size()) {
                        minIndex = i;
                    }
                }
                // 添加值
                updataKv.counter = notEmptyNum;
                resolueQuery[minIndex].push_back(updataKv);
                // 修改map
                map[notEmptyNum] = {minIndex, resolueQuery[minIndex].size() - 1};
            }
            notEmptyNum++;
            break;
        default:
            cout << "ERROR OP!";
            exit(-1);
            break;
        } 
    }

    volumeNumArr[preNotEmptyNum]--;
    // 判断是否超过目前volumeNumArr的极限,是就进行扩容
    if (notEmptyNum >= getVolumeNumArrLength()) {
        ExpandVNArr(); 
    }
    volumeNumArr[notEmptyNum]++;
    /**************************************** 判断并调整l ****************************************/
    uint32_t currMaxVolume = bemm->getMaxVolume();
    // 判断是否触发了l的变化
    if (notEmptyNum > currMaxVolume) {
        // 触发l变大
        bemm->setMaxVolume(notEmptyNum);
    } else if (preNotEmptyNum == currMaxVolume && volumeNumArr[preNotEmptyNum] == 0) {
        // l 变小
        for (int i=currMaxVolume; i>0; i--) {
            if (volumeNumArr[i] != 0) {
                bemm->setMaxVolume(i);
                break;
            }
        }
    }
    /**************************************** 判断并收缩EMM中元素至符合当前容量 ****************************************/
    // 调整EMMst
    ShrinkVolume(resolueQuery, key, (*EMMst)[key][ST_MAX_VOLUME]);
    SubmitUpdate(resolueQuery, key, preRandom);

    (*EMMst)[key][ST_SUBMIT_TIMES] = 0;
    (*EMMst)[key][ST_COALESCE_TIMES]++;
    (*EMMst)[key][ST_MAX_VOLUME] = bemm->getMaxVolume();

    return resolueQuery;
}

bool Client::ShrinkVolume(vector<vector<KV>> &resolueQuery, const char *key, int preVolume) {
    int currVolume = bemm->getMaxVolume();
    if ( preVolume > currVolume ) {    // 如果其 上次融合的时候的最大容量 大于 当前的最大容量, 就需要调整
        // 如果存在 有意义的value放在无意义的部分,那么就需要将其移动至有意义的部分
        bool changed = false;
        for (int i=currVolume; i < preVolume; i++) {   
            // 判断是否为该指纹的有意义的值
            for (int j = 0; j < resolueQuery[i].size(); j++) {
                if ( strcmp(resolueQuery[i][j].key, key) == 0) {
                    if (!resolueQuery[i][j].isPadding()) {   // 删除填充值
                        // 找到最短的位置,把值填充到那个位置
                        int minIndex = 0;
                        for (int p=1; p<currVolume; p++) {
                            if (resolueQuery[minIndex].size() > resolueQuery[i].size()) {
                                minIndex = i;
                            }
                        }
                        // 拷贝值
                        resolueQuery[minIndex].push_back(resolueQuery[i][j]);
                        changed = true;
                    }
                    resolueQuery[i].erase(resolueQuery[i].begin() + j);
                }
            }
        }
        return changed;
    } else {
        return false;
    }
}

void Client::SubmitUpdate(vector<vector<KV>> resolueQuery, const char *key, int *preRandom) {
    for (int i=0; i<resolueQuery.size(); i++) {
        ValueEntry ve(resolueQuery[i]);
        EncryptAndUpload(key, i, ve, preRandom[i]);
    }
}
#endif 