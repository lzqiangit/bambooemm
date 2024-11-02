#include <iostream>
#include "client.hpp"
#include "utils.hpp"
#include <vector>
#include "keyvaluetools.hpp"
using namespace std;

void Mapping()
{
    int n, l;
    vector<KV *> kvList = LoadKVList(n, l);
    Client *client = new Client();
    client->SetupEMM(kvList, n, l);
}

void QueryAfterMapping()
{

    int n, l;
    vector<KV *> kvList = LoadKVList(n, l);
    vector<int> volumnList = LoadVolumn();
    Client *client = new Client();
    cout << "初始化..." << endl;
    client->SetupEMM(kvList, n, l);

    cout << "初始化结束,准备查询!" << endl;
    BambooEMM *bemm = client->getBEMM();

    char *password = LoadKey();
    char *dec = new char[32];
    int decLen;
    int counter = 0;
    char *tempKey, *tempValue;
    int tempCounter, tempRandom;

    int moreTransCast = 0;
    for (int i = 0; i < 16384; i++)
    { // 16384
        string key = "key_" + to_string(i);
        vector<char *> values = bemm->Query((char *)key.c_str());
        moreTransCast += values.size() - l;

        for (int j = 0; j < values.size(); j++)
        {
            char *value = values.at(j);
            memset(dec, 0, 32);
            if (aes_decrypt_string(LoadKey(), value, BYTE_PER_VALUE, dec, &decLen) == -1)
            {
                cout << key << "|" << j << endl;
            }
        }
    }
    cout << "增加通信开销:" << moreTransCast << "|" << n << "(" << (float)moreTransCast / (float)(16384 * l) * 100.f << "%)" << endl;
}

void TestQueryRet()
{
    int n, l;
    vector<KV *> kvList = LoadKVList(n, l);
    vector<int> volumnList = LoadVolumn();
    Client *client = new Client();
    cout << "初始化..." << endl;
    client->SetupEMM(kvList, n, l);

    cout << "初始化结束,准备查询!" << endl;
    BambooEMM *bemm = client->getBEMM();

    char *password = LoadKey();
    char *dec = new char[32];
    int decLen;
    int counter = 0;
    char *tempKey, *tempValue;
    int tempCounter, tempRandom;

    int moreTransCast = 0;
    for (int i = 0; i < 16384; i++)
    { // 16384
        string key = "key_" + to_string(i);
        vector<char *> values = bemm->Query((char *)key.c_str());
        moreTransCast += values.size() - l;

        cout << key << ": ";
        for (int j = 0; j < values.size(); j++)
        {
            char *value = values.at(j);
            memset(dec, 0, 32);
            if (aes_decrypt_string(LoadKey(), value, BYTE_PER_VALUE, dec, &decLen) == -1)
            {
                cout << key << "|" << j << endl;
            }
            cout << dec << " @ ";
        }
        cout << endl
             << endl;
    }
    cout << "增加通信开销:" << moreTransCast << "|" << n << "(" << (float)moreTransCast / (float)(16384 * l) * 100.f << "%)" << endl;
}

void ReInsert()
{

    int n, l;
    vector<KV *> kvList = LoadKVList(n, l);
    vector<int> volumnList = LoadVolumn();
    Client *client = new Client();
    cout << "初始化..." << endl;
    client->SetupEMM(kvList, n, l);

    cout << "初始化结束,准备查询!" << endl;
    BambooEMM *bemm = client->getBEMM();

    char *password = LoadKey();
    char *dec = new char[32];
    int decLen;
    int counter = 0;
    char *tempKey, *tempValue;
    int tempCounter, tempRandom;

    string key = "key_" + to_string(1055);                // in
    // cout << "****************************************************" << endl;
    vector<char *> values = bemm->Query((char *)key.c_str());
    // cout << "****************************************************" << endl;
    vector<KV *> rKVList;
    char *tKey, *tval;
    int tcounter, trandom;
    cout << key << ": ";
    for (int j = 0; j < values.size(); j++)
    {
        char *value = values.at(j);
        memset(dec, 0, 32);
        if (aes_decrypt_string(LoadKey(), value, BYTE_PER_VALUE, dec, &decLen) == -1)
        {
            cout << key << "|" << j << endl;
        }
        cout << dec << " @ ";

        ResolveValue(dec, tKey, tcounter, tval, trandom);

        KV *tkv = new KV(tKey, tval, tcounter);
        tkv->random = trandom;
        rKVList.push_back(tkv);
    }
    cout << endl
         << endl;

    client->ReEncrypt(rKVList);

    values = bemm->Query((char *)key.c_str());
    cout << key << ": ";
    for (int j = 0; j < values.size(); j++)
    {
        char *value = values.at(j);
        memset(dec, 0, 32);
        if (aes_decrypt_string(LoadKey(), value, BYTE_PER_VALUE, dec, &decLen) == -1)
        {
            cout << key << "|" << j << endl;
        }
        cout << dec << " @ ";
    }
    cout << endl
         << endl;
}

void deleteValues(vector<char*> values) {
    for (char* value : values) {
        delete []value;
    }
}

void TestReInsertAll()
{
    int n, l;
    vector<KV *> kvList = LoadKVList(n, l);
    vector<int> volumnList = LoadVolumn();
    Client *client = new Client();
    cout << "初始化..." << endl;
    client->SetupEMM(kvList, n, l);

    cout << "初始化结束,准备查询!" << endl;
    BambooEMM *bemm = client->getBEMM();

    char *password = LoadKey();
    char *dec = new char[32];
    int decLen;
    int counter = 0;
    char *tempKey, *tempValue;
    int tempCounter, tempRandom;
    for (int i = 0; i <= 16383; i++)
    {
        string key = "key_" + to_string(i);
        cout << "****************************************************" << endl;
        vector<char *> values = bemm->Query((char *)key.c_str());
        cout << "****************************************************" << endl;
        vector<KV *> rKVList;
        char *tKey, *tval;
        int tcounter, trandom;
        cout << key << ": ";
        for (int j = 0; j < values.size(); j++)
        {
            char *value = values.at(j);
            memset(dec, 0, 32);
            if (aes_decrypt_string(LoadKey(), value, BYTE_PER_VALUE, dec, &decLen) == -1)
            {
                cout << key << "|" << j << endl;
            }
            cout << dec << " @ ";

            ResolveValue(dec, tKey, tcounter, tval, trandom);

            KV *tkv = new KV(tKey, tval, tcounter);
            tkv->random = trandom;
            rKVList.push_back(tkv);
        }
        deleteValues(values);

        client->ReEncrypt(rKVList);
        values = bemm->Query((char *)key.c_str());
        cout << endl << endl;
        cout << key << ": ";
        for (int j = 0; j < values.size(); j++)
        {
            char *value = values.at(j);
            memset(dec, 0, 32);
            if (aes_decrypt_string(LoadKey(), value, BYTE_PER_VALUE, dec, &decLen) == -1)
            {
                cout << key << "|" << j << endl;
            }
            cout << dec << " @ ";
        }
        cout << endl
             << endl;
        //cout << i << endl;
        rKVList.clear();
        deleteValues(values);
    }

    cout << "NOPROBLEM";
}

int main(int argc, char const *argv[])
{
    ReInsert();
    return 0;
}
