#include <iostream>
#include "UpdataEntry.hpp"
#include "utils.hpp"
#include "predefine.h"
#include "client.hpp"
using namespace std;

void testUpdataEntry()
{
    string value = "key_0|0|0,key_14563|9|219543";
    UpdataEntry ue(value.length() + 1, (char *)value.c_str(), OP_INSERT);
    string app = "key_0|6|6";
    ue.Append((char *)app.c_str());

    ue.SpliceRandom();

    cout << ue.getLen() << "\t" << ue.getP() << endl;

    ue.Enc(LoadKey());

    cout << ue.getLen() << "\t" << ue.getP() << endl;

    ue.Dec(LoadKey());
    cout << ue.getLen() << "\t" << ue.getP() << endl;

    ue.DivRandom();
    char* valueList = ue.DivValue();

    cout << valueList << "\t";

    cout << endl;
}

void testUplodAndGet() {
    int n, l;
    vector<KV *> kvList = LoadKVList(n, l);
    vector<int> volumnList = LoadVolumn();
    Client *client = new Client();
    cout << "初始化..." << endl;
    client->SetupEMM(kvList, n, l);

    cout << "初始化结束,准备查询!" << endl;
    BambooEMM *bemm = client->getBEMM();

    string updataValue = "key_u|0|uuu";
    char *password = LoadKey();
    vector<ValueEntry> vel;

    char *vp = (char*)updataValue.c_str();
    int len = strlen(vp);
    ValueEntry ve(len + 1, vp);
    for (int i = 0; i < 10; i++)
    { // 16384
        string key = "key_" + to_string(i);
        //client->Update((char*)key.c_str(), OP_DELETE, ve);        
        client->Query((char*)key.c_str());
    }
}

int main(int argc, char const *argv[])
{
    testUplodAndGet();  
    return 0;
}
