
#include <iostream>
#include "utils.hpp"

// ���ܵ�ʱ�����
// aes�е���Կ��ʽ AES_KEY
// ��װ����ʱ��ʹ�õ���Կ
using namespace std;
/**
 * �ҵ�Xor�����еļ��ܷ�����Ȼ����� k -> ek ��ƴ�Ӵ���BambooFilter��
 */

int main(int argc, char const *argv[])
{
    vector<KV *> kvList = LoadKVList();
    for (auto kv : kvList) {

    }

    unsigned char* key = AESGen(AES_KEY_LENGTH);
    cout << "key: " << key << endl;

    unsigned char* data = (unsigned char*)"hello world";
    cout << "data: " << data << endl;
    unsigned char* enc = AESEnc(key, data);
    cout << "enc: " << endl;
    unsigned char* des = AESDec(key, enc);
    cout << "des: " << des << endl; 
    return 0;
}
