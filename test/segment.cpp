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
    uint32_t add_count = 60000;
    vector<KV*> data;
    vector<char*> key;
    for (uint32_t i=0; i<add_count; i++) {
        char* temp_key = new char[5];
        memset(temp_key, 0, 5);
        memcpy(temp_key, &i, 4);
        for (uint32_t j=0; j<max_volumn; j++) {
            char *temp_value = new char[5];
            memset(temp_value, 0, 5);
            memcpy(temp_value, &j, 4);
            KV *kv = new KV(temp_key, temp_value, j);
            data.push_back(kv);
        }
        key.push_back(temp_key);
    }
    
    for (int i=0; i<data.size(); i++) {
        bemm.Insert(data[i]);
    }

// ²éÑ¯
    for (int i=0; i<key.size(); i++) {
        vector<char*> ret = bemm.Query(key.at(i));
        for (int j=0; j<ret.size(); j++) {
            cout << *(uint32_t*)ret[j] << "|"; 
        }
        cout << endl;
    }

}

int main()
{
    extend();
    return 0;
}
