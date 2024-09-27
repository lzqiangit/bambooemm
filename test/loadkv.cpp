#include <iostream>
#include "utils.hpp"
#include <vector>

using namespace std;

// ≤‚ ‘µº»Îkvlist
int main() {

    int n, l;
    vector<KV *> kvList = LoadKVList(n, l);
    for (KV *kv : kvList) {
        cout << kv->key << "|" << kv->value << "|" << kv->counter << endl;
    }
    cout << "Load KV: " << n << "; Max Volumn: " << l << endl;
    return 0; 
}