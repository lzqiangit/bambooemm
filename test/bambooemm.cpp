#include <iostream>
#include "bambooemm.hpp"
#include "KV.hpp"
#include "utils.hpp"
using namespace std;

unsigned char* makekey(unsigned char* key, int counter);

void setup() {
    BambooEMM bemm;
    int n, l;
    vector<KV *> mm = LoadKVList(n, l);
    
    bemm.setup(3, n/0.75, l, mm);
}

void lookup() {
    BambooEMM bemm;
    int n, l;
    vector<KV *> mm = LoadKVList(n, l);
    
    bemm.setup(3, n/0.75, l, mm);

    BambooFilter *bf = bemm.getEMM();


    string stre = "hello";
    cout << bf->Lookup(stre.c_str()) << endl;
    stre = "key_s_1";
    unsigned char* t0 = makekey((unsigned char*)stre.c_str(), 0);
    cout << t0 << endl;
    cout << "key_s_58,0 : " << bf->Lookup( (char*)t0 ) << endl;

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