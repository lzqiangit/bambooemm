#ifndef KV_H_
#define KV_H_


#include "BOBHash.h"
#include <string>
#include <cstring>
#include <vector>
#include <iostream>

#include "utils.hpp"
#include "Timer.hpp"

typedef unsigned int uint32_t;
using std::string;
using std::__cxx11::to_string;
using std::vector;
using std::cout;


class KV
{
public:
    char *key;
    char *value;
    int counter;

    KV();

    /**
     * 构建一个KV，传入key,counter和value
     * 注意构造kv后后对key和value的释放
     */
    KV(const char* key, int counter, const char* value);

    /**
     * 构建一个填充KV
     */
    KV(const char *key, int counter);

    /**
     * 移动构造函数
     */
    KV(KV&& others);

    ~KV();

    /**
     * 从key|counter|val的字符串中导入kv
     * 首先以 | 分割，key为第一个，counter为第二个，value为第三个
     * 注意counter需要转化为整形
     */
    KV(char *kcv);

    /**
     * 拷贝构造函数
     * 深拷贝
     */
    KV(const KV& others);

    KV& operator=(const KV& others);


    /**
     * 将成员变量以 key|counter|value 的形式拼接成字符串
     */
    char *Splice() const;

    /**
     * 获取其向服务端发送查询请求所需的key <- hash(key)|counter
     */
    char *QueryKey(uint32_t K);

    /**
     * 设置为填充值
     */
    void BePadding();

    bool isPadding();

    void setValue(const char *newValue);

    /**
     * 求key的BobHash哈希值
     */
    static string MakeHashKey(const char *key);
    /** 
     * 传入哈希后的key和counter,输出两者的拼接
     * 配合 KV::MakeHashKey 生成key的哈希值string
     * key <- hash(k)||c
     */
    static char *MakeSearchKey(string hashKey, int c);
    /**
     * 从key||counter||val的字符串列表中导入kv的列表
     */
    static vector<KV> LoadKVList(vector<char*> kvStrList);

private:
    char* copy_const_str(const char* cstr);
    int LenOfInt(int num);
};

#endif
