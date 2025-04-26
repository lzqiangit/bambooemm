#include "TwochClient.hpp"
#include "utils.hpp"
#include  "KV.hpp"


void sout(vector<KV> kvs) {

    cout << "key\tcounter\tvalue" << endl;
    for (auto kv : kvs) {
        cout << kv.key << "\t" << kv.counter << "\t" << kv.value << endl;
    }
}

int main(int argc, char const *argv[])
{
    TwochClient *client = new TwochClient();
    
    vector<KV*> kvList;
    int n, l;
    kvList = LoadKVList(n, l);


    client->SetupEMM(kvList, n, l);

    for (int i = 0; i < kvList.size(); i++) {
        delete kvList.at(i);
    }

    kvList.clear();

    cout << "初始化完成!!!" << endl;

    sout( client->Query("key_0") );

    cout << "+++++++++++++++++++++++++++++++++++++++" << endl;

    sout( client->Query("key_100") );


    delete client;
    return 0;
}
