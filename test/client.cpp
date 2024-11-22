#include <iostream>
#include "client.hpp"
#include "utils.hpp"
#include <vector>
#include <cstring>

using namespace std;

/*
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

void update() {
    int n, l;
    vector<KV *> kvList = LoadKVList(n, l);
    vector<int> volumnList = LoadVolumn();
    Client *client = new Client();
    cout << "初始化..." << endl;
    client->SetupEMM(kvList, n, l);

    string key = "key_0";
    string val = "null";
    client->Update((char*)key.c_str(), 0, OP_DELETE, (char*)val.c_str());
}

*/

void testReInsert()
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
    int decLen;
    int counter = 0;
    char *tempKey, *tempValue;
    int tempCounter, tempRandom;


    for (int i = 0; i < 16384; i++)
    { // 16384
        string key = "key_" + to_string(i);
        vector<ValueEntry> values = bemm->Query((char *)key.c_str());
        cout << "################################################################################" << endl;
        cout << key << ": " << endl;
        for (int j = 0; j < values.size(); j++)
        {
            ValueEntry value = values.at(j);
            char *dec = new char[value.getLen()];
            memset(dec, 0, 32);
            if (aes_decrypt_string(LoadKey(), value.getP(), value.getLen(), dec, &decLen) == -1)
            {
                cout << key << "|" << j << endl;
            }
            cout << dec << endl;
        }
        
    }
}


/**
 * 修改方案，将冲突key的value存储在同一给指纹entry的value中后测试程序是否能跑
 */
void TestBasFunction() {
    int n, l;
    vector<KV *> kvList = LoadKVList(n, l);
    vector<int> volumnList = LoadVolumn();
    Client *client = new Client();
    cout << "初始化..." << endl;
    client->SetupEMM(kvList, n, l);

    cout << "初始化结束,准备查询!" << endl;
    BambooEMM *bemm = client->getBEMM();

    char *password = LoadKey();
    int decLen;
    int counter = 0;
    char *tempKey, *tempValue;
    int tempCounter, tempRandom;


    for (int i = 0; i < 10; i++)
    { // 16384
        string key = "key_" + to_string(i);
        vector<ValueEntry> values = bemm->Query((char *)key.c_str());
        cout << "################################################################################" << endl;
        cout << key << ": " << endl;
        for (int j = 0; j < values.size(); j++)
        {
            ValueEntry value = values.at(j);
            char *dec = new char[value.getLen()];
            memset(dec, 0, 32);
            if (aes_decrypt_string(LoadKey(), value.getP(), value.getLen(), dec, &decLen) == -1)
            {
                cout << key << "|" << j << endl;
            }
            cout << dec << endl;
        }
        
    }
}

/**
 * 测试ValueEntry类的加密解密拼接等功能
 */
void testValueEntryFunction() {
    int n, l;
    vector<KV *> kvList = LoadKVList(n, l);
    vector<int> volumnList = LoadVolumn();
    Client *client = new Client();
    cout << "初始化..." << endl;
    client->SetupEMM(kvList, n, l);

    cout << "初始化结束,准备查询!" << endl;
    BambooEMM *bemm = client->getBEMM();

    char *password = LoadKey();
    int decLen;
    int counter = 0;
    char *tempKey, *tempValue;
    int tempCounter, tempRandom;


    for (int i = 0; i < 16384; i++)
    { // 16384
        string key = "key_" + to_string(i);
        vector<ValueEntry> values = bemm->Query((char *)key.c_str());
        cout << "################################################################################" << endl;
        cout << key << ": " << endl;
        for (int j = 0; j < values.size(); j++)
        {
            ValueEntry value = values.at(j);
            char *dec = new char[value.getLen()];
            memset(dec, 0, 32);
            if (aes_decrypt_string(LoadKey(), value.getP(), value.getLen(), dec, &decLen) == -1)
            {
                cout << key << "|" << j << endl;
            }
            cout << dec << endl;

            value.Dec(LoadKey());
            int random = value.DivRandom();
            vector<char*> valList = value.DivValue();
            for (char *val : valList) {
                cout << val << "\t";
            }
            cout << random << endl;
            cout << "------------------------------------------------------------------" << endl;
        }
        
    }
}

void testEncryptAndUpload() {
    int n, l;
    vector<KV *> kvList = LoadKVList(n, l);
    vector<int> volumnList = LoadVolumn();
    Client *client = new Client();
    cout << "初始化..." << endl;
    client->SetupEMM(kvList, n, l);

    cout << "初始化结束,准备查询!" << endl;
    BambooEMM *bemm = client->getBEMM();

    char *password = LoadKey();
    int decLen;
    int counter = 0;
    char *tempKey, *tempValue;
    int tempCounter, tempRandom;


    ValueEntry value;
    int random;
    vector<char*> valList;
    for (int i = 0; i < 10; i++)
    { // 16384
        string key = "key_" + to_string(i);
        vector<ValueEntry> values = bemm->Query((char *)key.c_str());
        cout << "######################################  BEFORE  ##########################################" << endl;
        cout << key << ": " << endl;
        for (int j = 0; j < values.size(); j++)
        {
            value = values.at(j);
            value.Dec(LoadKey());
            random = value.DivRandom();
            valList = value.DivValue();
            
            for (char *val : valList) {
                cout << val << "\t";
            }
            cout << random << endl;
            cout << "------------------------------------------------------------------" << endl;
            KV kv(valList[0]);
            ValueEntry newValue;
            newValue.SetValue(valList);
            client->EncryptAndUpload(kv.key, kv.counter,newValue, random);
        }


        values = bemm->Query((char *)key.c_str());
        cout << "######################################## AFTER ########################################" << endl;
        cout << key << ": " << endl;
        for (int j = 0; j < values.size(); j++)
        {
            value = values.at(j);
            value.Dec(LoadKey());
            random = value.DivRandom();
            valList = value.DivValue();
            
            for (char *val : valList) {
                cout << val << "\t";
            }
            cout << random << endl;
            cout << "------------------------------------------------------------------" << endl;
        }
    }
}

int main(int argc, char const *argv[])
{
    testEncryptAndUpload();
    return 0;
}
