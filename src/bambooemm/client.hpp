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
using std::pair;

class Client
{
private:
    /* data */
    int n;
    BambooEMM *bemm;
    uint32_t K, Ku;   // 种子
    unordered_map<string, uint32_t*>  *EMMst;


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
    void Update(char *key, uint32_t counter, char op, ValueEntry valueE);

    vector<KV> Query(const char *key);

private:
    uint32_t GetXHash(const char *key);
    /**
     * 拼接 操作对应的操作符和val
     */
    char *SpliceOpVal(char op, uint32_t counter, char *val); 
public:
    // 融合
    vector< vector<KV> > Coalesce(const char *key, int cnt, vector<ValueEntry> old);    
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
    for (int i = 0; i < kvList.size(); i++)
    {
        this->bemm->SetupInsert(kvList.at(i)); // counter必须从0开始而且连续 yes
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
            if (!(this->bemm->isExistKeyCounter(kv->key, kv->counter)))
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
 */
void Client::EncryptAndUpload(const char *key, int counter, ValueEntry valueE, int preRandom)
{
    char *hashKey = KV::MakeKey(key, counter);
    valueE.SpliceRandom(preRandom);
    valueE.Enc(LoadKey());
    bemm->ReInsert(hashKey, valueE);
}

/**
 * 更新EMMst中的数据,调用服务端添的接口,向EMMu中更新 key||counter位置的值
 * x <- hash(key||MMst[key][0], len, Ku);   // 用于计算EMMu中index的 中间值
 * y <- hash(MMst[kye][1]||x, len, Ku) % EMMu.length();     // EMMu的索引值
 * z <- Enc(Kenc, (op, counter,v));
 * EMMu[y] <- z
 * EMMst[label][1]++;
 */
void Client::Update(char *key, uint32_t counter, char op, ValueEntry valueE) {
    // 在st中找不到key,需要初始化
    if (EMMst->find(key) == EMMst->end()) {     
        (*EMMst)[key] = new uint32_t[2]{0, 0};
    }
    uint32_t x = GetXHash(key);
    // Question ！！！！！
    uint32_t y = GetYHash(x, (*EMMst)[key][1]);
    // 获取y
    UpdataEntry updataE(valueE, op);
    
    updataE.SpliceRandom();
    updataE.Enc(LoadKey());


    ++(*EMMst)[key][ST_SUBMIT_TIMES];
    // 上传服务器
    bemm->AddUpdata(y, updataE);
}

vector<KV> Client::Query(const char *key) {

    vector<ValueEntry> queryList = bemm->Query(key);
    vector< vector<KV> > resolueQuery;
    // 解密
    for (ValueEntry &query : queryList) {   // 使用引用才能真正实现queryList中元素的解密
        query.Dec(LoadKey());
    }
    uint32_t cnt = 0;
    if ( EMMst->find(key) != EMMst->end() ) {
        cnt = (*EMMst)[key][ST_SUBMIT_TIMES];
    }
    if (cnt > 0) {
        // 融合更新, 并解析结果
        resolueQuery = Coalesce(key, cnt, queryList);
    } else {
        for (int i=0; i<queryList.size(); i++) {
            ValueEntry ve = queryList.at(i);
            ve.DivRandom();
            // 解析value值
            vector<char*> values = ve.DivValue();
            vector<KV> kvs = KV::LoadKVList(values);
            resolueQuery.push_back(kvs);
        }
    }
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
    vector<ValueEntry> ret;
    uint32_t x = GetXHash(key);
    vector<UpdataEntry> ues = bemm->GetUpdataList(x, cnt);

    int queryLen = queryVEL.size();
    int *preRandom = new int[queryLen];
    int notEmptyNum = 0;     // 非空元素个数

    unordered_map<int, pair<int, int> > map;    // counter -> (ValueEntry_index, value_index)
    vector< vector<KV> > resolueQuery;      // 用于存储解析的查询结果, 并在其上进行更新操作, 
    // 解析查询结果, 找到每个 key||counter 对应的 qeryVEL 的索引, 并记录下随机数
    for (int i=0; i<queryLen; i++) {
        ValueEntry ve = queryVEL.at(i);
        preRandom[i] = ve.DivRandom();    // 记录随机数
        // 解析value值
        vector<char*> values = ve.DivValue();
        vector<KV> kvs = KV::LoadKVList(values);
        resolueQuery.push_back(kvs);
        // 遍历判断
        for (int j=0; j<kvs.size(); j++) {
            KV kv = kvs.at(j);
            if ( strcmp( kv.key, key) == 0) {
                map[kv.counter] = {i, j};       // 还可能有找不到的counter,这种counter就是共用了其他value的值！
                if (!kv.isPadding()) {
                    notEmptyNum++;
                }
            }
        }
    }
    // 解析更新
    for (UpdataEntry &updata : ues) {
        updata.Dec(LoadKey());
        updata.DivRandom();
        char *value = updata.DivValue();
        KV updataKv(value);                                     // 更新后的值
        char op = updata.DivOP();

        int upCounter = updataKv.counter;
        pair<int, int> pos = map[upCounter];
        KV aimKv = resolueQuery.at(pos.first).at(pos.second);   // 需要修改的

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
                                // 需要修改map吗?
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
            break;
        default:
            cout << "ERROR OP!";
            exit(-1);
            break;
        } 
    }
    // 提交更新
    for (int i=0; i<resolueQuery.size(); i++) {
        ValueEntry ve(resolueQuery[i]);
        EncryptAndUpload(key, i, ve, preRandom[i]);
    }

    return resolueQuery;
}
#endif