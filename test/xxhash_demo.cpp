#include <iostream>
#include <cstring>
#include <xxhash.h>
#include "Timer.hpp"
#include "BOBHash.h"

int main() {
    // 输入数据
    const char* data = "pppppppppppppppkey_0";
    size_t data_len = strlen(data);

    // 定义种子
    uint32_t seed = 12335;

    Timer::getInstance().start();
    // 计算哈希值
    uint32_t hash = XXH32(data, data_len, seed);
    Timer::getInstance().stop();
    std::cout << "查询时间:" << Timer::getInstance().getDuration() << "ms" << std::endl;
    // 输出结果
    std::cout << "Hash: " << hash << std::endl;

    // 将hash 同 ,1 进行拼接
    std::string hashStr = std::to_string(hash);
    hashStr += "," + std::to_string(1);

    const char *hashCStr = hashStr.c_str();
    uint32_t hashLen = strlen(hashCStr);

    Timer::getInstance().start();
    // 计算哈希值
    hash = BOBHash::run(hashCStr, hashLen);
    Timer::getInstance().stop();
    std::cout << "查询时间:" << Timer::getInstance().getDuration() << "ms" << std::endl;
    // 输出结果
    std::cout << "Hash: " << hash << std::endl;

    return 0;
}