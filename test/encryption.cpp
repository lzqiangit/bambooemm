
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
char* join(char *key, char *value, int counter);
int main(int argc, char const *argv[])
{
    string keyStr = "key_727";
    string valueStr = "10775";
    int counter = 11;
    string passwordStr = "dfasfa";
    char *password = (char*)passwordStr.c_str();


    char *kvc = join((char*)keyStr.c_str(), (char*)valueStr.c_str(), counter);

    char *enc = new char[32];
    int encLen;
    int ret = aes_encrypt_string(password, kvc, strlen(kvc), enc, &encLen);

    char *dec = new char[32];
    cout << encLen << "|" << strlen(enc) << endl;
    int decLen;
    cout << strlen(enc) << endl;
    int len = strlen(enc) <= 16 ? 16 : 32;
    aes_decrypt_string(password, enc, len, dec, &decLen);

    cout << dec << "|" << strlen(dec) << endl;

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

char* join(char *key, char *value, int counter) {
    string keyStr = key;
    string valueStr = value;
    
    string ret = keyStr + '|' + valueStr + '|' + to_string(counter);
    cout << ret << endl;
    char *retCStr = new char[ret.length() + 1];
    memset(retCStr, 0, ret.length() + 1);
    memcpy(retCStr, (char*)ret.c_str(), ret.length());
    return retCStr;
}

void test() {
    
    string keyStr = "helloaes";
    char *key = (char*)keyStr.c_str();


    string inStr = "key_727|10775|11";
    char *in = (char*)inStr.c_str();
    char *enc = new char[4096];
    int encLen;

    cout << "key:" << key << endl;
    cout << "data:" << in << "|" << strlen(in) + 1<< endl;
    int ret = aes_encrypt_string(key, in, strlen(in), enc, &encLen);
    cout << "enc:" << enc << "|" << encLen << endl;

    char *dec = new char[4096];
    int decLen;
    ret = aes_decrypt_string(key, enc, encLen, dec, &decLen);
    cout << "dec:" << dec << "|" << decLen + 1 << endl;
    /**
     * 密文长度总之会填充至16的整数倍
     */
    cout << sizeof(char*) << endl;

}
