#include <iostream>
#include <cstring>
#include <xxhash.h>

int main() {
    // 输入数据
    const char* data = "Hello, xxHash!";
    size_t data_len = strlen(data);

    // 定义种子
    uint32_t seed = 12335;

    // 计算哈希值
    uint32_t hash = XXH32(data, data_len, seed);

    // 输出结果
    std::cout << "Hash: " << std::hex << hash << std::endl;

    return 0;
}