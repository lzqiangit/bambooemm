#include <iostream>
#include "bambooemm.hpp"
#include "KV.hpp"
#include "utils.hpp"
#include "common/random.h"
using namespace std;

unsigned char* makekey(unsigned char* key, int counter);

void setup() {
    BambooEMM bemm;
    int n, l;
    vector<KV *> mm = LoadKVList(n, l);
    
    bemm.Setup(3, n/0.75, l);
}

void insert() {
    
}

void lookup() {
    BambooEMM bemm;
    int n, l;
    vector<KV *> mm = LoadKVList(n, l);
    
    bemm.Setup(3, n/0.75, l);
    bemm.LoadMM(mm);

    string key_str = "key_s_10";
    const char* key = key_str.c_str();
    vector<char*> ret = bemm.Query(key);

    uint32_t *temp = new uint32_t;
    for (int i=0; i<ret.size(); i++) {
        memcpy(temp, ret.at(i), BYTE_PER_VALUE);
        cout << *temp << endl;
    }
}

int main(int argc, char const *argv[])
{
 
    lookup();
    return 0;
}


unsigned char* makekey(unsigned char* key, int counter) {

    unsigned char* KI = LoadKey();
    cout << "KI: " << KI << endl;
    unsigned char *encKey = AESEnc(KI, key);
    int encKeyLen = strlen((char *)encKey);
    int tailLen = LenOfInt(counter) + 2;
    char *tail = new char[tailLen];
    sprintf(tail, ",%d", counter);
    unsigned char *tailUS = (unsigned char *)tail;  // 指针赋值，不能提前释放tail！

    int ktLen = encKeyLen + tailLen;
    unsigned char *kt = new unsigned char[ktLen];
    memset(kt, 0, ktLen);
    memcpy(kt, encKey, encKeyLen);
    memcpy(kt + encKeyLen, tailUS, tailLen);
    return kt;
}