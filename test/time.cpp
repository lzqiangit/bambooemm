#include "utils.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "client.hpp"
#include "Timer.hpp"
using namespace std;

void ShowKVList(vector<KV> kvs);
void SaveToCSV(vector<pair<int, double>> timeList, string filename, string title);

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
}

// 测试获取时间戳和计算时间差的函数
// void testBaseFun() {
//     auto star = getCurTimePoint();

//     //this_thread::sleep_for(std::chrono::seconds(1));
//     int sum = 10;
//     for (int i=0; i<100; i++) {
//         sum += i;        
//     }

//     auto end = getCurTimePoint();

//     cout << getTimeDiff(star, end) << endl;
// }


// // 测试不同n下面, 关键字的实际容量对查询时间的影响
// void testRealLAndQueryTime() {

//     int MAX_KEY_INDEX = 4064;
//     vector<pair<int, double>> timeList;
//     string key;
//     for (int i=0; i<=MAX_KEY_INDEX; i++) {
//         key = "key_";
//         key += to_string(i);
//         key = string(20 - key.length(), 'p') + key;
//         auto star = getCurTimePoint();
//         vector<KV> kvs = client->Query(key.c_str());
//         auto end = getCurTimePoint();

//         timeList.push_back(make_pair(kvs.size(), getTimeDiff(star, end)));
//         // cout << getTimeDiff(star, end) << endl;
//         // ShowKVList(kvs);
//     }

//     SaveToCSV(timeList, "volumn_query_time_20_9.csv", "l,time");
// }

// 测试平均的查询时间


void tesAverageQuertyTime() {

    int MAX_KEY_INDEX = 1033;
    double sum = 0;
    int times = 0;
    string key;
    //for (int i=0; i<=MAX_KEY_INDEX; i++) {
        key = "ppppppppppppkey_1055";
        // key += to_string(1);
        // key = string(20 - key.length(), 'p') + key;

        cout << "开始" << endl;
        //Timer::getInstance().start();
        vector<KV> kvs = client->Query(key.c_str());
        //Timer::getInstance().stop();
        //sum += getTimeDiff(star, end);
        //times++;
        // cout << getTimeDiff(star, end) << endl;
        // ShowKVList(kvs);
    //}
    //cout << "查询时间:" << Timer::getInstance().getDuration() << "ms" << endl;
    ShowKVList(kvs);
    //cout << "平均查询时间: " << sum / times << endl;
}

int main() {

    // testBaseFun();
    Init();
    tesAverageQuertyTime();
    delete client;
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