#include "utils.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "client.hpp"
using namespace std;

void ShowKVList(string key, vector<string> vals) {
    int counter = 0;
    cout << endl;
    cout << "=================================================================" << endl;
    cout << "KEY" << "\t" << "COUNTER" << "\t" << "VALUE" << endl;
    cout << "-----------------------------------------------------------------" << endl;
    for (auto val : vals) {
        cout << key << "\t" << counter++ << "\t" << val << endl;
    }
    cout << "=================================================================" << endl;
    cout << endl;
}

int n, l;
Client *client;

void Init() {
    vector<KV *> kvList;
    cout << "导入数据..." << endl;
    kvList = LoadKVList(n, l);
    cout << "成功导入!!! (" << n << "条数据" << ",最大容量为:" << l << ")"<< endl;

    cout << "初始化EMM..." << endl;
    client = new Client();
    client->SetupEMM(kvList, n, l);
    cout << "初始化完成!!!" << endl;
    // 清除kvList
    for (auto kv : kvList) {
        delete kv;
    }

     kvList.clear();
}

int main() {

    Init();

    vector<pair<int, double>> timeList;
    for (int i=0; i<=267; i++) {    // TODO
        string key = "key_" + to_string(i);
        client->Query(key.c_str());
    }
    SaveToCSV(client->mCsvData, "query_bamboo_n22_l15.csv", "key,time"); // TODO

    delete client;
    return 0;
}



