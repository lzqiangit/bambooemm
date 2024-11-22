#include <iostream>
#include "UpdataEntry.hpp"
#include "utils.hpp"
#include "predefine.h"
#include "client.hpp"
using namespace std;

ValueEntry TEMP_VALUE(20 ,"key_temp|0|val_temp");

void showMenu() {
    cout << "查询:S\t" << "插入:I\t" << "删除:D\t" << "修改:E\t" << endl;
    cout << "清屏:C\t" << "退出:X\t" <<endl;  
}

char* InputCStr(string name) {
    string val;
    cout << name + ":";
    cin >> val;
    int len = val.length();
    char *valCStr = new char[len + 1];
    memset(valCStr, 0, len + 1);
    memcpy(valCStr, (char*)val.c_str(), len);
    return valCStr;
}

int InputNum(string name) {
    int num;
    cout << name + ":";
    cin >> num;
    return num;
}

void ShowKVList(vector<KV> kvs) {
    cout << "=================================================================" << endl;
    cout << "KEY" << "\t" << "COUNTER" << "\t" << "VALUE" << endl;
    cout << "-----------------------------------------------------------------" << endl;
    for (auto kv : kvs) {
        cout << kv.key << "\t" << kv.counter << "\t" << kv.value << endl;
    }
    cout << "=================================================================" << endl;
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
    char *key;
    int counter;
    char *value;
    ValueEntry valE;
    while (true) {
        key = nullptr;
        value = nullptr;
        cout << "$ ";
        cin >> comm;
        switch (comm)
        {
        case 'S':
        case 's':
            key = InputCStr("key");
            ShowKVList(client->Query(key));
            break;
        case 'I':
        case 'i':
            key = InputCStr("key");
            counter = InputNum("counter");
            value = InputCStr("value");
            valE.SetValue(key, counter, value);
            client->Update(key, counter, OP_INSERT, valE);
            break;
        case 'D':
        case 'd':
            key = InputCStr("key");
            counter = InputNum("counter");
            client->Update(key, counter, OP_DELETE, TEMP_VALUE);
            break;
        case 'E':
        case 'e':
            key = InputCStr("key");
            counter = InputNum("counter");
            value = InputCStr("value");
            valE.SetValue(key, counter, value);
            client->Update(key, counter, OP_DELETE, valE);
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
        default:
            cout << "命令错误" << endl;
            showMenu();
            break;
        }

        if (key != nullptr)    delete[] key;
    }

    return 0;
}
