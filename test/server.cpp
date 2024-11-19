#include <iostream>
#include "UpdataEntry.hpp"
#include "utils.hpp"
#include "predefine.h"
using namespace std;


int main(int argc, char const *argv[])
{
    string value = "key_0|0|0,key_14563|9|219543";
    UpdataEntry ue(value.length() + 1, (char*)value.c_str(), OP_INSERT);
    string app = "key_0|6|6";
    ue.Append((char*)app.c_str());

    ue.SpliceRandom();

    cout << ue.getLen() << "\t" << ue.getP() << endl;

    ue.Enc(LoadKey());

    cout << ue.getLen() << "\t" << ue.getP() << endl;

    ue.Dec(LoadKey());
    cout << ue.getLen() << "\t" << ue.getP() << endl;
    
    ue.DivRandom();     
    vector<char*> valueList = ue.DivValue();
    
    for (char *val : valueList) {
        cout << val << "\t";
    }


    cout << endl;




    return 0;
}
