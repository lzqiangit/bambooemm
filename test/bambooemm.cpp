// #include <iostream>
// #include "bambooemm.hpp"
// #include "KV.hpp"
// #include "utils.hpp"
// #include "common/random.h"
// using namespace std;

// unsigned char* makekey(unsigned char* key, int counter);

// void setup() {
//     BambooEMM bemm;
//     int n, l;
//     vector<KV *> mm = LoadKVList(n, l);
    
//     bemm.Setup(3, n/0.75, l, LoadKey());
// }

// void insert() {
    
// }

// void lookup() {
//     BambooEMM bemm;
//     int n, l;
//     vector<KV *> mm = LoadKVList(n, l);
    
//     bemm.Setup(3, n/0.75, l, LoadKey());
//     //bemm.LoadMM(mm);

//     string key_str = "key_s_10";
//     const char* key = key_str.c_str();
//     vector<char*> ret = bemm.Query(key);

//     uint32_t *temp = new uint32_t;
//     for (int i=0; i<ret.size(); i++) {
//         memcpy(temp, ret.at(i), BYTE_PER_VALUE);
//         cout << *temp << endl;
//     }
// }

// void Random_() {
//     BambooEMM bemm;
//     for (int i=0; i<10; i++) {
//         //cout << bemm.RandomNumStr(2, 32) << endl;
//     }
// }

// void ResolveValue() {
//     int n, l;
//     vector<KV*> kvList = LoadKVList(n, l);
//     BambooEMM bemm;
//     //char *spliceValue = bemm.SpliceValue(kvList[1024]);
//     //cout << spliceValue << endl;
//     char *key, *value;
//     int counter, random;
//     //bemm.ResolveValue(spliceValue, key, counter, value, random);
//     cout << key << endl << counter << endl << value << endl << random << endl;
// }

// int main(int argc, char const *argv[])
// {
 
//     ResolveValue();
//     return 0;
// }
int main() {
    return 0;
}