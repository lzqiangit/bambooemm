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
