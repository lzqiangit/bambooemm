#include <iostream>
#include "utils.h"
#include "kv.h"

using namespace std;

int main() {
    cout << "STAR" << endl;
    kv* kvList;
    int len = LoadKVList(kvList);
    cout << "��ȡ��¼���ȣ�" << endl;
    cout << "RRRRRRRRRRRUNING" << endl;
    return 0; 
}