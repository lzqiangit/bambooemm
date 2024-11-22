#ifndef KV_H_
#define KV_H_


#include "BOBHash.h"
#include <string>
#include <cstring>
#include <vector>

typedef unsigned int uint32_t;
using std::string;
using std::__cxx11::to_string;
using std::vector;


class KV
{
public:
    char *key;
    char *value;
    int counter;

    KV(char *key, int counter) {
        this->key = new char[strlen(key) + 1];
        memset(this->key, 0, strlen(key) + 1);
        memcpy(this->key, key, strlen(key));

        this->value = nullptr;
        BePadding();
    }

    KV(char *key, char *value, int counter) :counter(counter)
    {
        this->key = new char[strlen(key) + 1];
        memset(this->key, 0, strlen(key) + 1);
        memcpy(this->key, key, strlen(key));

        this->value = new char[strlen(value) + 1];
        memset(this->value, 0, strlen(value) + 1);
        memcpy(this->value, value, strlen(value));
    }
    ~KV()
    {
        delete key;
        delete value;
    }

    KV(char *kcv) {
        string spliceValueStr = kcv;
        int star = 0;
        int end = spliceValueStr.find('|');
        string substring = spliceValueStr.substr(star, end - star);
        key = copy_const_str(substring.c_str());

        star = end + 1;
        end = spliceValueStr.find('|', star);
        char *counterCStr = copy_const_str(spliceValueStr.substr(star, end - star).c_str());
        counter = atoi(counterCStr);

        star = end + 1;
        end = spliceValueStr.length();
        value = copy_const_str(spliceValueStr.substr(star, end - star).c_str());
    }

    KV(const KV& others) {
        char *keyo = others.key;
        char *valueo = others.value;
        this->key = new char[ strlen(keyo) + 1 ];
        this->value = new char[ strlen(valueo) + 1 ];
        memset(this->key, 0, strlen(keyo) + 1);
        memset(this->value, 0, strlen(valueo) + 1);
        memcpy(this->key, keyo, strlen(keyo));
        memcpy(this->value, valueo, strlen(valueo));
        this->counter = others.counter;
    }

    KV& operator=(const KV& others) {
        if (this->key != nullptr) {
            delete []this->key;
        }
        if (this->value != nullptr) {
            delete []this->value;
        }
        char *keyo = others.key;
        char *valueo = others.value;
        this->key = new char[ strlen(keyo) + 1 ];
        this->value = new char[ strlen(valueo) + 1 ];
        memset(this->key, 0, strlen(keyo) + 1);
        memset(this->value, 0, strlen(valueo) + 1);
        memcpy(this->key, keyo, strlen(keyo));
        memcpy(this->value, valueo, strlen(valueo));
        this->counter = others.counter;
        return *this;
    }

    /**
     * 获取kcv的拼接 key|counter|value
     */
    char *Splice() {
        string keyStr = key;
        string valueStr = value;
        int len = keyStr.length() + valueStr.length() + LenOfInt(counter) + 2;
        int padLen = 0;
        string ret = keyStr + '|' + to_string(counter) + "|" + valueStr;        // + "|" + RandomNumStr(RANDOM_NUM_LEN, retRandom)   
        int retLen = ret.length();
        char *retCStr = new char[retLen + 1];
        memset(retCStr, 0, retLen + 1);
        memcpy(retCStr, (char *)ret.c_str(), retLen);
        return retCStr;
    }

    /**
     * 获取其向服务端发送查询请求所需的key <- hash(key)|counter
     */
    char *QueryKey() {
        return MakeKey(this->key, this->counter);
    }

    /**
     * 设置为填充值
     */
    void BePadding() {
        if (this->value != nullptr)    delete[] this->value;
        this->value = new char[2];
        memset(this->value, 0, 2);
        sprintf(this->value, "P");
    }

    bool isPadding() {
        return (strlen(this->value) == 1 && this->value[0] == 'P'); 
    }

    void setValue(char *newValue) {
        int newLen = strlen(newValue);
        if (this->value != nullptr) {
            delete[] this->value;
        }
        this->value = new char[newLen + 1];
        memset(this->value, 0, newLen + 1);
        memcpy(this->value, newValue, newLen);
    }

    /**
     * 通过key和counter获取用于查询的关键字 
     * key <- hash(k)||c
     */
    static char *MakeKey(const char *k, int c) {
        uint32_t hash_key = BOBHash::run(k, strlen(k), 3);
        
        string keyStr = to_string(hash_key);
        string counterStr = to_string(c);
        string keyCounterStr = keyStr + "|" + counterStr;
        int len = keyCounterStr.length();
        char *retCStr = new char[len + 1];
        memset(retCStr, 0, len + 1);
        memcpy(retCStr, (char *)keyCounterStr.c_str(), len);
        return retCStr;
    }
    /**
     * 从key||counter||val的字符串列表中导入kv的列表
     */
    static vector<KV> LoadKVList(vector<char*> kvStrList) {
        vector<KV> ret;
        for (char * kvStr : kvStrList) {
            KV kv(kvStr);
            ret.push_back(kv);
        }
        return ret;
    }

private:
    char* copy_const_str(const char* cstr) {
        int len = strlen(cstr);
        char* cpy = new char[len + 1];
        memset(cpy, 0, len + 1);
        memcpy(cpy, cstr, len);
        return cpy;
    }
    int LenOfInt(int num) {
        int len = 1;
        while (num >= 10) {
            ++len;
            num /= 10;
        }
        return len;
    }
};

#endif
