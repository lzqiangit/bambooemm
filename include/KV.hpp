#ifndef KV_H_
#define KV_H_


#include "BOBHash.h"
#include <string>
#include <cstring>
typedef unsigned int uint32_t;
using std::string;
using std::__cxx11::to_string;


class KV
{
public:
    char *key;
    char *value;
    int counter;
    KV(char *key, char *value, int counter) : key(key),
                                              value(value),
                                              counter(counter)
    {
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
