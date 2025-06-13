#include "utils.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "TwochClient.hpp"
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
TwochClient *client;

void Init() {
    vector<KV *> kvList;
    cout << "导入数据..." << endl;
    kvList = LoadKVList(n, l);
    cout << "成功导入!!! (" << n << "条数据" << ",最大容量为:" << l << ")"<< endl;

    cout << "初始化EMM..." << endl;
    client = new TwochClient();

    client->SetupEMM(kvList, n, l);

    cout << "TwoCh初始化耗时:" << Timer::getInstance().getDuration() << endl;
    // 计算存储空间

    // 清除kvList
    for (auto kv : kvList) {
        delete kv;
    }
    kvList.clear();
}

int main() {

    Init();

    Update update = Update('I', "new_value", 9);

    int times = 14;
    double uploadTime = 0;
    double queryTime = 0;
 
    for (int i=0; i<=times; i++) {
        string key = "key_" + to_string(i);
        Timer::getInstance().start();
        client->UploadUpdate(key.c_str(), update);
        Timer::getInstance().stop();
        uploadTime += Timer::getInstance().getDuration();

        queryTime += client->Query(key.c_str());
    }
    cout << "上传耗时:" << uploadTime/times << endl;
    cout << "融合耗时:" << queryTime/times << endl;
    

    // ret = client->Query(key);
    // ShowKVList(key, ret);

    delete client;
    return 0;
}



