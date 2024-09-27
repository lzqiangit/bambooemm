
#include <iostream>
#include "utils.hpp"

// 加密的时候调用
// aes中的秘钥格式 AES_KEY
// 封装加密时候使用的秘钥
using namespace std;
/**
 * 找到Xor代码中的加密方法，然后加密 k -> ek 后拼接存入BambooFilter中
 */
void test();
int main(int argc, char const *argv[])
{
    test();
    // unsigned char* key = AESGen(AES_KEY_LENGTH);
    // cout << "key: " << key << endl;

    // unsigned char* data = (unsigned char*)"hello world";
    // cout << "data: " << data << endl;
    // unsigned char* enc = AESEnc(key, data);
    // cout << "enc: " << endl;
    // unsigned char* des = AESDec(key, enc);
    // cout << "des: " << des << endl; 
    return 0;
}

void test() {
    unsigned char* key = AESGen(3);
    
    for (int i=0; i<1024; i++) {
        unsigned char* data = new unsigned char[6 + LenOfInt(i) + 1];
        sprintf((char*)data,"key_s_%d", i);
        cout << AESDec(key, AESEnc(key, data)) << endl;
        delete data;
    }
}
