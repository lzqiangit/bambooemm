#include <iostream>
#include "UpdataEntry.hpp"
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
    KV *kcv;
    ValueEntry valE;

    showMenu();
    while (true) {
        kcv = nullptr;

        cout << "BEMM$ ";
        cin >> comm;
        switch (comm)
        {
        case 'S':
        case 's':
            keyStr = InputStr("key");
            ShowKVList(client->Query((char*)keyStr.c_str()));
            break;
        case 'I':
        case 'i':
            keyStr = InputStr("key");
            valStr = InputStr("value");
            kcv = new KV(keyStr, PADDING_COUNTER, valStr);
            client->Update((char*)keyStr.c_str() , OP_INSERT, *kcv);
            break;
        case 'D':
        case 'd':
            keyStr = InputStr("key");
            counter = InputNum("counter");
            kcv = new KV(keyStr, counter, PADDING_VALUE);
            client->Update((char*)keyStr.c_str(), OP_DELETE, *kcv);
            break;
        case 'E':
        case 'e':
            keyStr = InputStr("key");
            counter = InputNum("counter");
            valStr = InputStr("value");
            kcv = new KV((char*)keyStr.c_str(), counter, valStr);
            client->Update((char*)keyStr.c_str(), OP_EDIT, *kcv);
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
        default:
            cout << "命令错误" << endl;
            showMenu();
            break;
        }
        if (kcv != nullptr) delete kcv;
    }

    return 0;
}
