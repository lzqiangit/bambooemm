#ifndef CLIENT_H_
#define CLIENT_H_

#include "bambooemm.hpp"
#include "utils.hpp"
#include "predefine.h"
#include "filterposition.hpp"

#include <unordered_map>
#include <string>
#include <sstream>

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
    void MappingStep(vector<KV *> kvList, int l);
    BambooEMM *getBEMM();
    /**
     * 此函数用于更新操作,再融合完成value完成后,给value添加随机数,加密后上传服务器更新valueEntry
     * key 需要重写的key
     * valueE 需要重写的未添加随机数且未加密状态的valueEntry
     * preRandom 该value的前一个随机数 
     */
    void EncryptAndUpload(char *key, ValueEntry valueE, int preRandom);

    /**
     * 更新函数: 
     * key: 需要更新的key <- hash(key)||counter
     * counter
     * op: 需要对 key[counter]执行的操作
     * value: 操作后的值 insert,edit需要value而delete不需要
     */
    void Update(char *key, uint32_t counter, char op, ValueEntry valueE);

private:
    char *SpliceX(char *key, int st0);
    char *SpliceY(uint32_t emm1, uint32_t x);
    /**
     * 拼接 操作对应的操作符和val
     */
    char *SpliceOpVal(char op, uint32_t counter, char *val); 

    // 融合
    void Coalesce();    
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
    MappingStep(maxCounterKVList, l);
    // 加密
    bemm->AddRandomAndEncrypt(LoadKey());
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
            KV *kv = new KV(key, "0", ++counter);
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



void Client::EncryptAndUpload(char *key, ValueEntry valueE, int preRandom)
{
    valueE.SpliceRandom(preRandom);
    valueE.Enc(LoadKey());
    bemm->ReInsert(key, valueE);
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
    char *spliceX = SpliceX(key, (*EMMst)[key][0]);
    uint32_t x = BOBHash::run(spliceX, strlen(spliceX), Ku);
    // Question ！！！！！
    char *spliceY = SpliceY((*EMMst)[key][1], x);
    uint32_t y = BOBHash::run(spliceY, strlen(spliceY), Ku);  // 这里直接按照char*处理???      // 这个哈希的种子只能是质数？？ 不用了
    // 获取y
    char *opv = SpliceOpVal(op, counter, valueE.getP());        // 长度？？？
    // 加密
    char *encOpv = new char[32];
    int encLen;
    aes_encrypt_string(LoadKey(), opv, strlen(opv) + 1, encOpv, &encLen); 
    char *decOpv = new char[32];
    int decLen;
    aes_decrypt_string(LoadKey(), encOpv, encLen, decOpv, &decLen);

    char *dec = new char[decLen + 1];
    memset(dec, 0, decLen + 1);
    memcpy(dec, decOpv, decLen);

    delete []opv;
    delete []encOpv;
    delete []decOpv;

    //cout << decOpv << endl;       # 函数返回报错 Seg fault!!!!!!
    // 上传服务器
}

/**********************************************  Splice  *******************************************************************/
char *Client::SpliceX(char *key, int st0) {
    string keyStr = key;
    string st0Str = to_string(st0);
    string xStr = keyStr + "|" + st0Str; 
    char *ret = new char[xStr.length() + 1];
    memset(ret, 0, xStr.length() + 1);
    memcpy(ret, (char*)xStr.c_str(), xStr.length());
    return ret;
}

char *Client::SpliceY(uint32_t emm1, uint32_t x) {
    string emm1Str = to_string(emm1);
    string xStr = to_string(x);
    string splice = emm1Str + "|" + xStr;
    char *ret = new char[splice.length() + 1];
    memset(ret, 0, splice.length() + 1);
    memcpy(ret, (char*)splice.c_str(), splice.length());
    return ret;
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

void Client::Coalesce() {

}
#endif