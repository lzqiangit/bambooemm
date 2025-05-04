#ifndef _VALUE_ENTRY_H_
#define _VALUE_ENTRY_H_

typedef unsigned int uint32_t;
#include <cstring>
#include <string>
#include "utils.hpp"
#include <vector>
#include <random>
#define RANDOM_MAX 99999999
#define RANDOM_MAX_LEN 9

using std::uniform_int_distribution;
using std::random_device;
using std::mt19937;

class KV;

class ValueEntry
{
public:
    int len;
    char *p;
    ValueEntry();

    ValueEntry(int len, char *p);

    ValueEntry(const ValueEntry &other);

    ValueEntry(const KV &kv);

    ValueEntry(vector<KV> kvs);

    ValueEntry(ValueEntry &&other) noexcept;

    virtual ~ValueEntry();

    void SetValue(KV kv);

    void SetValue(int len, char *p);

    /**
     * 将value设置为 key|counter|value
     */
    void SetValue(char *key, int counter, char *value);

    void SetValue(vector<char *> values);

    char *DuplicateValue(int &len, char *dv);

    void CpFrom(ValueEntry ve);
    /**
     * 在value后拼接字符串
     */
    void Append(const char *append);

    void AppendValue(const char *append);

    void erase();

    /**
     * 加密values
     */
    bool Enc(const char *password);
    /**
     * 解密values
     */
    bool Dec(const char *password);

    /**
     * 在values后面添加random
     */
    void SpliceRandom(int pre = 0);
    /**
     * 剔除values后面的random
     */
    int DivRandom();

    /**
     * 以 逗号(,) 为标志, 分割values为value数组
     */
    vector<char *> DivValue();

    int getLen() const;
    char *getP() const;

    ValueEntry &operator=(ValueEntry &ve);

    size_t getMemOverhead();

    bool isEmpty() const;
};
#endif