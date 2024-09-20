#include <iostream>
#include "utils.h"
#include "kv.h"

using namespace std;

int main() {
    cout << "STAR" << endl;
    kv* kvList;
    int len = LoadKVList(kvList);
    cout << "读取记录长度：" << endl;
    cout << "RRRRRRRRRRRUNING" << endl;
    return 0; 
}