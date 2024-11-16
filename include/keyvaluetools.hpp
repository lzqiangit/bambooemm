#ifndef _KEY_VALUE_TOOLS_H_
#define _KEY_VALUE_TOOLS_H_

#include "utils.hpp"
#include "predefine.h"
#include <iostream>
#include <string>
#include <random>


char *SpliceKey(uint32_t key, int counter);
char *SpliceValue(KV *kv);
void ResolveValue(char *spliceValue, char *&key, int &counter, char *&value, int &random);
/**
 * 获取一个与pre不相等的长度为len的数字,并将其转为char*返回
 */
//char *RandomNumStr(int len, int pre);



/**
 * 将key同counter拼接，返回拼接后的字符串
 * 注意不会将key哈希
 */
char *SpliceKey(uint32_t key, int counter)
{
    string keyStr = to_string(key);
    string counterStr = to_string(counter);
    string keyCounterStr = keyStr + "|" + counterStr;
    char *retCStr = new char[keyCounterStr.length() + 1];
    memset(retCStr, 0, keyCounterStr.length() + 1);
    memcpy(retCStr, (char *)keyCounterStr.c_str(), keyCounterStr.length());
    return retCStr;
}

/**
 * 返回的长度为segment中value单元的规定值
 */
char *SpliceValue(KV *kv)
{

    string keyStr = kv->key;
    string valueStr = kv->value;

    int len = keyStr.length() + valueStr.length() + LenOfInt(kv->counter) + 2;
    int padLen = 0;

    string ret = keyStr + '|' + to_string(kv->counter) + "|" + valueStr;        // + "|" + RandomNumStr(RANDOM_NUM_LEN, retRandom)
    
    int retLen = ret.length();
    char *retCStr = new char[BYTE_PER_VALUE];
    memset(retCStr, 0, BYTE_PER_VALUE);
    memcpy(retCStr, (char *)ret.c_str(), retLen);
    return retCStr;
}

void ResolveValue(char *spliceValue, char *&key, int &counter, char *&value, int &random)
{
    string spliceValueStr = spliceValue;
    int star = 0;
    int end = spliceValueStr.find('|');
    string substring = spliceValueStr.substr(star, end - star);
    key = copy_const_str(substring.c_str());

    star = end + 1;
    end = spliceValueStr.find('|', star);
    char *counterCStr = copy_const_str(spliceValueStr.substr(star, end - star).c_str());
    counter = atoi(counterCStr);

    star = end + 1;
    end = spliceValueStr.find('|', star);
    value = copy_const_str(spliceValueStr.substr(star, end - star).c_str());

    star = end + 1;
    end = spliceValueStr.find('|', star);
    char *randomCStr = copy_const_str(spliceValueStr.substr(star, end - star).c_str());
    random = atoi(randomCStr);
}

// char *RandomNumStr(int len, int pre)
// {
//     int next = pre;
//     random_device rd;
//     mt19937 gen(rd());
//     uniform_int_distribution<> dis(pow(10, len - 1), pow(10, len) - 1);

//     do
//     {
//         next = dis(gen);
//     } while (pre == next);
//     char *nextStr = new char[len + 1];
//     memset(nextStr, 0, len + 1);
//     sprintf(nextStr, "%d", next);
//     return nextStr;
// }




#endif