#include "utils.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "client.hpp"
using namespace std;

void ShowKVList(vector<KV> kvs);
void SaveToCSV(vector<pair<int, double>> timeList, string filename, string title);

int n, l;
vector<KV *> kvList;
Client *client;

void Init() {
    
    cout << "导入数据..." << endl;
    kvList = LoadKVList(n, l);
    cout << "成功导入!!! (" << n << "条数据" << ",最大容量为:" << l << ")"<< endl;

    cout << "初始化EMM..." << endl;
    client = new Client();
    client->SetupEMM(kvList, n, l);
    cout << "初始化完成!!!" << endl;
}

// 测试获取时间戳和计算时间差的函数
void testBaseFun() {
    auto star = getCurTimePoint();

    this_thread::sleep_for(std::chrono::seconds(1));

    auto end = getCurTimePoint();

    cout << getTimeDiff(star, end) << endl;
}


// 测试查询时间
void testQueryTime() {

    int MAX_KEY_INDEX = 4089;
    vector<pair<int, double>> timeList;
    string key;
    for (int i=0; i<=MAX_KEY_INDEX; i++) {
        key = "key_";
        key += to_string(i);
        key = string(20 - key.length(), 'p') + key;
        auto star = getCurTimePoint();
        vector<KV> kvs = client->Query(key.c_str());
        auto end = getCurTimePoint();

        timeList.push_back(make_pair(kvs.size(), getTimeDiff(star, end)));
        // cout << getTimeDiff(star, end) << endl;
        // ShowKVList(kvs);
    }

    SaveToCSV(timeList, "volumn_query_time_20_9.csv", "l,time");

    
}


int main() {

    Init();
    testQueryTime();
    return 0;
}


void ShowKVList(vector<KV> kvs) {
    cout << endl;
    cout << "=================================================================" << endl;
    cout << "KEY" << "\t" << "COUNTER" << "\t" << "VALUE" << endl;
    cout << "-----------------------------------------------------------------" << endl;
    for (auto kv : kvs) {
        cout << kv.key << "\t" << kv.counter << "\t" << kv.value << endl;
    }
    cout << "=================================================================" << endl;
    cout << endl;
}

void SaveToCSV(vector<pair<int, double>> timeList, string filename, string title) {
    ofstream outfile;
    outfile.open("/home/lzq/code/bambooemm/analyse/csv/" + filename);
    outfile << title << endl;
    for (auto t : timeList) {
        outfile << t.first << "," << t.second << endl;
    }
    outfile.close();
}