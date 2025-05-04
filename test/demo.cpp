#include <iostream>
#include "UpdateEntry.hpp"
#include "utils.hpp"
#include "predefine.h"
#include "client.hpp"
using namespace std;

#define PADDING_COUNTER 0
#define PADDING_VALUE "P"

// key_16383 -> key_16384

void showMenu() {
    cout << "=================================================================" << endl;
    cout << "查询:S\t" << "插入:I\t" << "删除:D\t" << "修改:E\t" << endl;
    cout << "容量:V\t" << "消耗:O\t"  <<endl;  
    cout << "清屏:C\t" << "退出:X\t" <<endl;  
    cout << "=================================================================" << endl;
}

char* InputCStr(string name) {
    string val;
    //cout << name + ": ";
    cin >> val;
    int len = val.length();
    char *valCStr = new char[len + 1];
    memset(valCStr, 0, len + 1);
    memcpy(valCStr, (char*)val.c_str(), len);
    return valCStr;
}

string InputStr(string name) {
    string val;
    //cout << name + ": ";
    cin >> val;
    return val;
}

int InputNum(string name) {
    int num;
    //cout << name + ":";
    cin >> num;
    return num;
}

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

int main(int argc, char const *argv[])
{

    int n, l;
    cout << "导入数据..." << endl;
    vector<KV *> kvList = LoadKVList(n, l);
    cout << "成功导入!!! (" << n << "条数据" << ",最大容量为:" << l << ")"<< endl;

    cout << "初始化EMM..." << endl;
    Client *client = new Client();
    client->SetupEMM(kvList, n, l);
    cout << "初始化完成!!!" << endl;

    char comm;
    int counter;
    string keyStr;
    string valStr;
    string value;
    ValueEntry valE;

    showMenu();
    while (true) {
        cout << "BEMM$ ";
        cin >> comm;
        unordered_map<string, unordered_map<string, size_t>> overhead;
        switch (comm)
        {
        case 'S':
        case 's':
            keyStr = InputStr("key");
            ShowKVList(keyStr, client->Query(keyStr.c_str()));
            break;
        case 'I':
        case 'i':
            keyStr = InputStr("key");
            valStr = InputStr("value");
            client->Update(keyStr.c_str() , Update(OP_INSERT, valStr.c_str(), valStr.size()));
            break;
        case 'D':
        case 'd':
            keyStr = InputStr("key");
            
            client->Update((char*)keyStr.c_str(), Update(OP_DELETE, valStr.c_str(), valStr.size()));
            break;
        case 'C':
        case 'c':
            system("clear");
            break;
        case 'X':
        case 'x':
            delete client;
            exit(0);
            break;
        case 'M':
        case 'm':
            showMenu();
            break;
        case 'V':
        case 'v':
            cout << "MaxVolume:" <<  client->getBEMM()->getMaxVolume() << endl;
            break;
        case 'O':
        case 'o':
            client->getMemOverHead();

            break;
        default:
            cout << "命令错误" << endl;
            showMenu();
            break;
        }
    }

    return 0;
}
