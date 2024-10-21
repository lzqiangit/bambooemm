#include <iostream>
#include "client.hpp"
#include "utils.hpp"
#include <vector>
using namespace std;

void Mapping() {
    int n, l;
    vector<KV*> kvList = LoadKVList(n, l);
    Client *client = new Client();
    client->SetupEMM(kvList, n, l);
}

void QueryAfterMapping() {

    int n, l;
    vector<KV*> kvList = LoadKVList(n, l);
    Client *client = new Client();
    cout << "初始化..." << endl;
    client->SetupEMM(kvList, n, l);

    cout << "初始化结束,准备查询!" << endl;
    BambooEMM *bemm = client->getBEMM();

    char *password = LoadKey();
    char *dec = new char[32];
    int decLen;
    for (int i=0; i<1024; i++) {
        string key = "key_" + to_string(i);
        cout << key << " : ";
        
        vector<char*> values = bemm->Query((char*)key.c_str());
        for (int i=0; i<values.size(); i++) {
            char *value = values.at(i);
            memset(dec, 0, 32);
            aes_decrypt_string(LoadKey(), value, BYTE_PER_VALUE, dec, &decLen);
            // KV *kv = new KV((char*)key.c_str(), (char*)to_string(counter++).c_str(), i);
            // char *aim = SpliceValue(kvList.at(counter++));
            // if (strcmp(aim, dec) != 0) {
            //     cout << key << ":" << dec << "\t aim: " << aim << endl;
            //     --counter;
            // }
            cout << dec << " # ";   
        }
        cout << endl;
    }
}

int main(int argc, char const *argv[])
{
    QueryAfterMapping();   
    return 0;
}
