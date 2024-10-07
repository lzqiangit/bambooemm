#include <iostream>
#include "bambooemm.hpp"
#include "KV.hpp"
#include "utils.hpp"
#include "common/random.h"
using namespace std;

void Extend(Segment *src, int act_bit, int table_size)
{
    Segment *dst = new Segment(*src);

    uint32_t num_seg_bits_ = (uint32_t)ceil(log2((double)table_size));
    int num_table_bits_ = num_seg_bits_ + BUCKETS_PER_SEG;
    src->EraseEle(true, act_bit - 1);
    dst->EraseEle(false, act_bit - 1);
}


void extend() {
    BambooEMM bemm;
    uint32_t max_volumn = 30;
    bemm.Setup(3, 1000, max_volumn);
    uint32_t add_count = 273;           // extend会有出现0的情况，不extend只会出现错误的value 273
    vector<KV*> data;
    vector<char*> key;
    for (uint32_t i=0; i<add_count; i++) {
        char* temp_key = new char[5];
        memset(temp_key, 0, 5);
        memcpy(temp_key, &i, 4); 
        for (uint32_t j=0; j<max_volumn; j++) {
            char *temp_value = new char[5];
            memset(temp_value, 0, 5);
            uint32_t temp_v = (max_volumn * i) + j;
            memcpy(temp_value, (char*)(&temp_v), 4);
            KV *kv = new KV(temp_key, temp_value, j);
            data.push_back(kv);
        }
        key.push_back(temp_key);
        
    }
    
    for (int i=0; i<data.size(); i++) {
        if (i == 164) {
            cout << endl;
        }
        if (i == 2541) {
            cout << endl;
        }
        bemm.Insert(data[i]);
    }

// 查询
    uint32_t counter = 0;
    for (int i=0; i<key.size(); i++) {
        vector<char*> ret = bemm.Query(key.at(i));
        cout << i << "\t" ;
        for (int j=0; j<ret.size(); j++) {
            cout << *(uint32_t*)ret.at(j);
            // if (*(uint32_t*)ret[j] != counter++) {
            //     cout << "<ERROR> " << j;
            //     counter--;
            // }
                
            cout <<  "|"; 
            
        }
        cout << endl;
    }

}

int main()
{
    extend();
    return 0;
}
