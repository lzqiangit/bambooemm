#ifndef CLIENT_H_
#define CLIENT_H_

#include "bambooemm.hpp"
#include "utils.hpp"

class Client
{
private:
    /* data */
    BambooEMM *bemm;

public:
    Client(/* args */);
    ~Client();

    void SetupEMM(vector<KV*> kvList, int n, int l);
    void MappingStep(vector<KV*> kvList, int l);
    BambooEMM *getBEMM();
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
void Client::SetupEMM(vector<KV*> kvList, int n, int l) {
    vector<KV*> maxCounterKVList;           // 存储每个key中counter最大的元素
    this->bemm = new BambooEMM();
    this->bemm->Setup(2, n/0.75, l, LoadKey());
    char *tempKey = kvList.at(0)->key;
    for (int i=0; i<kvList.size(); i++) {
        this->bemm->Insert(kvList.at(i));             // counter必须从0开始而且连续 yes
        if (i == kvList.size() - 1) {
            maxCounterKVList.push_back(kvList.at(i));
            continue;
        }
        if (strcmp(tempKey, kvList.at(i + 1)->key) != 0) {
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
void Client::MappingStep(vector<KV*> kvList, int l) {
    int counter = 0;
    int passCounter = 0;
    char *key;
    for (KV *kv : kvList) {
        counter = kv->counter;    
        key = kv->key;
        for (int i=counter; i < l; i++) {
            KV *kv = new KV(key, "00000", ++counter);
            if (!(this->bemm->isExistKeyCounter(kv->key, kv->counter))) {
                this->bemm->Insert(kv);
            } else {
                cout << "OKKKKKKKKKKKK! : " << kv->key << "||" << kv->counter << endl;
                ++passCounter;
            }
            //delete kv;            // ? 泄露?????
        }
    } 
    cout << "PASS:" << passCounter << endl;
}   

BambooEMM* Client::getBEMM() {
    return this->bemm;
}

#endif