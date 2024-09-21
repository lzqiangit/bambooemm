#include <iostream>
#include "utils.hpp"
#include <vector>

using namespace std;

int main() {

    vector<KV *> kvList = LoadKVList();
    // for (KV kv : kvList) {
    //     cout << kv.key << "|" << kv.value << "|" << kv.counter << endl;
    // }
    // for (int i=0; i<kvList.size(); i++) {
    //     KV kv = kvList[i];
    //     cout << kv.key << "|" << kv.value << "|" << kv.counter << endl;
    // }
    return 0; 
}