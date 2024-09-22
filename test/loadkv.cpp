#include <iostream>
#include "utils.hpp"
#include <vector>

using namespace std;

// ���Ե���kvlist
int main() {

    vector<KV *> kvList = LoadKVList();
    for (KV *kv : kvList) {
        cout << kv->key << "|" << kv->value << "|" << kv->counter << endl;
    }
    return 0; 
}