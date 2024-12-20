#include <immintrin.h>
#include <stdint.h>
#include <stdlib.h>

#include <iostream>
#include "bitsutil.h"
#include "predefine.h"
#include "utils.hpp"
#include "ValueEntry.hpp"

#include <stdlib.h>

using namespace std;

class Segment
{
private:
    // const
    static const uint32_t kTagsPerBucket = 4;
    static const uint32_t kBytesPerBucket = (BITS_PER_TAG * kTagsPerBucket + 7) >> 3;

    static const uint32_t kTagMask = (1ULL << BITS_PER_TAG) - 1;
    static const uint64_t kBucketMask = (1ULL << (BITS_PER_TAG * kTagsPerBucket)) - 1;

    static const uint32_t bucket_size = (BITS_PER_TAG * kTagsPerBucket + 7) / 8; // kBytesPerBucket
    static const uint32_t safe_pad = sizeof(uint64_t) - bucket_size;
    static const uint32_t safe_pad_simd = 4; // 4B for avx2

private:
    char *temp;
    const uint32_t chain_num;   // 段中的桶的数量
    uint32_t chain_capacity; // 溢出链 + 1（初始段）
    uint32_t total_size;
    uint32_t insert_cur;
    char *data_base;
    uint32_t ANS_MASK;
    ValueEntry **value_set;

    static uint32_t IndexHash(uint32_t index)
    {
        return index & ((1 << BUCKETS_PER_SEG) - 1);
    }
    static uint32_t AltIndex(size_t index, uint32_t tag)
    {
        return IndexHash((uint32_t)(index ^ tag));
    }

    /**
     * p 桶的起始地址
     * idx tag编号(0~3)
     * tag 需要写入的指纹
     */
    static void WriteTag(char *p, uint32_t idx, uint32_t tag)
    {
        uint32_t t = tag & kTagMask;
        p += (idx + (idx >> 1));
        if ((idx & 1) == 0)
        {
            ((uint16_t *)p)[0] &= 0xf000;
            ((uint16_t *)p)[0] |= t;
        }
        else
        {
            ((uint16_t *)p)[0] &= 0x000f;
            ((uint16_t *)p)[0] |= (t << 4);
        }
    }

    static uint32_t ReadTag(const char *p, uint32_t idx)
    {
        uint32_t tag;
        p += idx + (idx >> 1);
        tag = *((uint16_t *)p) >> ((idx & 1) << 2);
        return tag & kTagMask;
    }

    static bool LookupTag(const char *p, uint32_t tag)
    {
        uint64_t v = *((uint64_t *)p);
        return hasvalue12(v, tag);
    }

    /**
     * p : 桶的起始地址
     * idx : tag的index
     * old_tag : 需要删除的tag
     */
    static bool RemoveOnCondition(const char *p, uint32_t idx, uint32_t old_tag)
    {
        p += idx + (idx >> 1);
        uint32_t tag = (*((uint16_t *)p) >> ((idx & 1) << 2)) & kTagMask;
        if (old_tag != tag & kTagMask)
        {
            return false;
        }

        if ((idx & 1) == 0)
        {
            ((uint16_t *)p)[0] &= 0xf000;
        }
        else
        {
            ((uint16_t *)p)[0] &= 0x000f;
        }
        return true;
    }

    /**
     * p : 桶的起始地址
     * tag : 需要删除的tag
     */
    static bool DeleteTag(char *p, uint32_t tag)
    {
        for (size_t tag_idx = 0; tag_idx < kTagsPerBucket; tag_idx++)
        {
            if (RemoveOnCondition(p, tag_idx, tag))
            {
                return true;
            }
        }
        return false;
    }

    /**
     * p 起始地址
     * is_src true则留0 false则留1
     * actv_bit num_table_bits_ - INIT_TABLE_BITS ， num_table_bits_ = num_seg_bits_ + BUCKETS_PER_SEG
     */
    static uint64_t doErase(char *p, bool is_src, uint32_t actv_bit)
    {
        uint64_t v = (*(uint64_t *)p) & kBucketMask;

        ((uint64_t *)p)[0] &= 0xffff000000000000;
        // ((uint64_t *)p)[0] |= is_src ? ll_isl(v, actv_bit) : ll_isn(v, actv_bit);

        uint64_t delFlag = (v & (0x001001001001ULL << actv_bit)) >> actv_bit;
        //cout << delFlag << endl;
        //cout << v << endl;
        if (is_src) {
            ((uint64_t *)p)[0] |= (v & (~(delFlag * 0xFFFULL)));
            //cout << (v & (~(delFlag * 0xFFFULL))) << endl;
        } else {
            ((uint64_t *)p)[0] |= (v & (delFlag * 0xFFFULL));
            //cout << (v & ((delFlag * 0xFFFULL)));
        }
        return delFlag;
    }

    /**
     * 擦除指定位置的value_set中指向(ValueEntry的指针)
     */
    void eraseValueEP(uint32_t bucket_id, uint32_t chain_id, uint32_t tag_id) {
        int index = ((bucket_id * chain_capacity) + chain_id) * kTagsPerBucket + tag_id;
        value_set[index] = nullptr;
    }

    /**
     * 擦除value
     * delFlag 删除标识符 ？？？ 
     */
    bool eraseValue(uint32_t bucket_id, uint32_t chain_id, uint64_t delFlag, bool is_src) {
        uint64_t mask = 0x000000000001ULL;
        for (int i=0; i<kTagsPerBucket; i++) {
            ValueEntry *valueP = get_value(bucket_id, chain_id, i);
            if (delFlag & mask) {
                if (is_src) {
                    valueP->erase();
                }
            } else {
                if (!is_src) {
                    valueP->erase();
                }
            } 
            mask = mask << BITS_PER_TAG;
        } 
        return true;         
    }

    static __m256i unpack12to16(const char *p)
    {
        __m256i v = _mm256_loadu_si256((const __m256i *)(p - 4)); // 即为从p-4开始导入32个字节数据到v中，为什么要移动四个？   256/12 =

        const __m256i bytegrouping =
            _mm256_setr_epi8(4, 5, 5, 6, 7, 8, 8, 9, 10, 11, 11, 12, 13, 14, 14, 15,
                             0, 1, 1, 2, 3, 4, 4, 5, 6, 7, 7, 8, 9, 10, 10, 11);
        v = _mm256_shuffle_epi8(v, bytegrouping);

        __m256i hi = _mm256_srli_epi16(v, 4);                            // // 以16为单位，逻辑右移，注意x86小端对齐
        __m256i lo = _mm256_and_si256(v, _mm256_set1_epi32(0x00000FFF)); // 按位与 32, 注意大小端对齐 11111111 00001111 00000000 00000000

        return _mm256_blend_epi16(lo, hi, 0b10101010); // 第三个参数从右向左代表复制到的目标数值的从低到高位
                                                       // 0表示复制第一个参数的对应位置，1表示复制第二个参数的对应位置
    }

    void SetValue(uint32_t bucket_id, uint32_t chain_id, uint32_t tag_id, ValueEntry valueE)
    {
        // if (bucket_id == 25 && chain_id == 1 && tag_id == 0) {
        //     cout << endl;
        // }
        ValueEntry *value_p = get_value(bucket_id, chain_id, tag_id);
        value_p->CpFrom(valueE);
    }

    void AppendValue(uint32_t bucket_id, uint32_t chain_id, uint32_t tag_id, ValueEntry valueE) {
        ValueEntry *value_p = get_value(bucket_id, chain_id, tag_id);
        value_p->AppendValue(valueE.getP());
    }

    /**
     * 获取value的指针
     */
    ValueEntry *get_value(uint32_t bucket_id, uint32_t chain_id, uint32_t tag_id) const
    {
        int index = ((bucket_id * chain_capacity) + chain_id) * kTagsPerBucket + tag_id;
        ValueEntry *ret = value_set[index];
        return ret;   
    }

    bool isCrash(char* bucket_p, uint32_t tag) {
        uint32_t t = tag & kTagMask;
        for (int idx = 0; idx<4; idx++) {

            bucket_p += idx + (idx >> 1);
            uint32_t tagp = (*((uint16_t *)bucket_p) >> ((idx & 1) << 2)) & kTagMask;
            if (tagp == t)
            {
                return true;
            } 
            
        }
        return false;
    }

    /**
     * 判断valueP位置是否为空
     */
    bool isEmptyValue(char *valueP) {
        return *((uint32_t*)valueP) == 0;
    }

private:
    /**
     * 通过对对比结果获取value的地址指针
     * 被LookupP调用的私有函数
     * cmp 对比的结果
     * times 16一组的比较次数
     * chain_idx 桶编号（Look传入的，未AltIndex前的，该桶及其平行溢出链排在前面）
     * tag 指纹
     */
    ValueEntry *LookupValueP(uint32_t cmp, int times, size_t chain_idx, uint32_t tag) const{
        vector<char*> ret;
        vector<int> tag_indexs = cmp_to_tag_id(cmp);
        if (tag_indexs.size() > 2) {                               // 后面没问题再简化操作！！！！！！！！！！！！！！！！！！！！！！！！！！！！直接通过一部计算得到index
            cout << "查询到两个相同的指纹存在于同一个查询路径上！！！ - 1" << endl;
            exit(-1);
        }

        if (tag_indexs.size() == 2) {
            int bucket_id = 0, chain_id = 0, tag_id = 0;
            for (int i=0; i<2; i++) {
                int tag_index = tag_indexs[i];
                tag_index += times * 16;
                // 计算一个segment中
                int a_chains_tag_num = chain_capacity * kTagsPerBucket;
                if (tag_index < a_chains_tag_num)
                {
                    bucket_id ^= (int)chain_idx;
                }
                else
                {
                    tag_index -= a_chains_tag_num;
                    bucket_id ^= AltIndex(chain_idx, tag);
                }
                chain_id ^= tag_index / kTagsPerBucket; // 一个桶中4个tag
                tag_id ^= tag_index % kTagsPerBucket;
                ValueEntry *valueE = get_value(bucket_id, chain_id, tag_id);
                //cout << valueE->getP() << endl;
            }
            if (bucket_id != 0 || chain_id != 0 || tag_id != 0) {
                cout << "查询到两个相同的指纹存在于同一个查询路径上！！！ - 2" << endl;
                exit(-1);
            }    
        }

        if (tag_indexs.size() == 0) {
            return nullptr;
        }

        int tag_index = tag_indexs[0];
        char *value = NULL;

        tag_index += times * 16;

        // 计算一个segment中
        int a_chains_tag_num = chain_capacity * kTagsPerBucket;
        int bucket_id, chain_id, tag_id;
        if (tag_index < a_chains_tag_num)
        {
            bucket_id = (int)chain_idx;
        }
        else
        {
            tag_index -= a_chains_tag_num;
            bucket_id = AltIndex(chain_idx, tag);
        }
        chain_id = tag_index / kTagsPerBucket; // 一个桶中4个tag
        tag_id = tag_index % kTagsPerBucket;
        ValueEntry *valueE = get_value(bucket_id, chain_id, tag_id);
        return valueE;  
    }

    /**
     * 查找指向value的指针，外部调用接口
     * chain_idx : bucket_index
     */
public:
    ValueEntry *LookupP(uint32_t chain_idx, uint16_t tag) const
    {
        memcpy(temp + safe_pad_simd,
               data_base + chain_idx * chain_capacity * bucket_size, // bucket_size = 6
               chain_capacity * bucket_size);
        memcpy(temp + safe_pad_simd + chain_capacity * bucket_size,
               data_base + AltIndex(chain_idx, tag) * chain_capacity * bucket_size,
               chain_capacity * bucket_size);

        //char *value_set_p_0 = value_set + chain_idx * chain_capacity * kTagsPerBucket * BYTE_PER_VALUE; // 桶的起始地址
        //char *value_set_p_1 = value_set + AltIndex(chain_idx, tag) * chain_capacity * kTagsPerBucket * BYTE_PER_VALUE;

        char *p = temp + safe_pad_simd;
        char *end = p + 2 * chain_capacity * bucket_size;

        int times = 0;
        __m256i _true_tag = _mm256_set1_epi16(tag); // 将tag装入16个平行的16字节中（p标量）
        uint32_t cmp = 0;
        bool ret = false;  
        while (p + 24 <= end)                       // 一次查 24*8/12 = 16个 也就是四个桶       24*8 = 192
        {
            /**
             * 把16个tag分别填充到16*16bit中，每个tag的12bit占据低
             */
            __m256i _16_tags = unpack12to16(p); // 一个tag 12bits，8*24/12 = 16个tag

            __m256i _ans = _mm256_cmpeq_epi16(_16_tags, _true_tag);
            cmp = _mm256_movemask_epi8(_ans);
            if (cmp)
            {
                return LookupValueP(cmp, times, chain_idx, tag);
                ret = true;
            }
            p += 24;
            ++times;
        }
        __m256i _16_tags = unpack12to16(p);
        __m256i _ans = _mm256_cmpeq_epi16(_16_tags, _true_tag);
        cmp = ANS_MASK & _mm256_movemask_epi8(_ans);
        if (cmp)
        {
            return LookupValueP(cmp, times, chain_idx, tag);
        }
        return nullptr;
    }

public:
    Segment(const uint32_t chain_num)
        : chain_num(chain_num),
          chain_capacity(1),
          insert_cur(0),
          ANS_MASK(~(0xFFFFFFFF << 2 * (2 * chain_capacity * kTagsPerBucket % 16)))
    {
        total_size = chain_num * chain_capacity * bucket_size + safe_pad;
        data_base = new char[total_size];
        memset(data_base, 0, (chain_num * chain_capacity * bucket_size));
        temp = new char[safe_pad_simd + (2 * chain_capacity * bucket_size + 23) / 24 * 24 + safe_pad_simd]; // temp前填充 safepad 的4byte   *2：两个候选桶
        // 初始化value_set
        value_set = new ValueEntry*[getTagNum()];
        for (int i=0; i<getTagNum(); i++) {
            value_set[i] = new ValueEntry();
        }
    }

    Segment(const Segment &s)
        : chain_num(s.chain_num),
          chain_capacity(s.chain_capacity),
          total_size(s.total_size),
          insert_cur(0),
          ANS_MASK(s.ANS_MASK)
    {
        data_base = new char[total_size];
        temp = new char[safe_pad_simd + (2 * chain_capacity * bucket_size + 23) / 24 * 24 + safe_pad_simd];
        memcpy(data_base, s.data_base, total_size);

        value_set = new ValueEntry*[getTagNum()];
        
        for (int i=0; i<getTagNum(); i++) {
            value_set[i] = new ValueEntry(s.getValue(i));
        }
        
        // 深拷贝对象,深拷贝！！！
    }

    ~Segment()
    {
        delete[] data_base;
        delete[] temp;
        delete[] value_set;
    };

    /**
     * 该方法用于第一次插入 k-v
     * chain_idx 桶id
     * curtag 指纹
     * valueE 需要插入的ValueEntry
     */
    bool Insert(uint32_t chain_idx, uint32_t curtag, ValueEntry valueE)
    {
        char *bucket_p;
        ValueEntry *value_p;
        for (uint32_t count = 0; count < MAX_CUCKOO_KICK; count++)
        {
            bucket_p = data_base + (chain_idx * chain_capacity + insert_cur) * bucket_size;

            bool kickout = count > 0;
            for (size_t tag_idx = 0; tag_idx < kTagsPerBucket; tag_idx++)
            {
                if ( (0 == ReadTag(bucket_p, tag_idx)))
                {
                    WriteTag(bucket_p, tag_idx, curtag);
                    // 写入value
                    SetValue(chain_idx, insert_cur, tag_idx, valueE);
                    return true;
                }
            }
            if (kickout)
            {
                size_t tag_idx = rand() % kTagsPerBucket;
                uint32_t oldtag = ReadTag(bucket_p, tag_idx);
                WriteTag(bucket_p, tag_idx, curtag);
                value_p =  get_value(chain_idx, insert_cur, tag_idx);
                SwapValue(&valueE, value_p);
                curtag = oldtag;
            }
            chain_idx = AltIndex(chain_idx, curtag);
            bucket_p = data_base + (chain_idx * chain_capacity + insert_cur) * bucket_size;
        }

        insert_cur++;
        if (insert_cur >= chain_capacity)
        {
            char *old_data_base = data_base;
            uint32_t old_chain_len = chain_capacity * bucket_size;
            // 计算初始valueset的长度
            uint32_t old_valueset_len = chain_capacity * kTagsPerBucket;
            chain_capacity++;
            uint32_t new_chain_len = chain_capacity * bucket_size;
            ANS_MASK = ~(0xFFFFFFFF << 2 * (2 * chain_capacity * kTagsPerBucket % 16));
            delete[] temp;
            temp = new char[safe_pad_simd + (2 * chain_capacity * bucket_size + 23) / 24 * 24 + safe_pad_simd];

            total_size = chain_num * chain_capacity * bucket_size + safe_pad;
            data_base = new char[total_size];
            memset(data_base, 0, total_size);

            // 重新复制value
            ValueEntry **new_value_set = new ValueEntry*[getTagNum()];

            uint32_t new_valueset_len = chain_capacity * kTagsPerBucket;    // 每个bucket及其桶中有多少给value

            for (int i = 0; i < chain_num; i++)
            {
                memcpy(data_base + i * new_chain_len, old_data_base + i * old_chain_len, old_chain_len);
                // 因为这里相当于移动，只复制地址，不用再深拷贝后删除原来的
                memcpy(new_value_set + i * new_valueset_len, value_set + i * old_valueset_len, old_valueset_len * sizeof(ValueEntry*));
                for (int p=0; p<kTagsPerBucket; p++) {
                    ValueEntry *vp =  new ValueEntry();
                    new_value_set[i*new_valueset_len+old_valueset_len+p] = vp;
                }
            }
            delete[] old_data_base;
            delete[] value_set;
            value_set = new_value_set;
        }
        return Insert(chain_idx, curtag, valueE);
    }  

    /**
     * chain_idx : bucket_index
     */
    bool Lookup(uint32_t chain_idx, uint16_t tag, ValueEntry &ve) const
    {
        ValueEntry *valueP = LookupP(chain_idx, tag);
        if (valueP == nullptr) {
            return false;
        }
        ve.CpFrom(*valueP);
        return true;
    }

    bool Delete(uint32_t chain_idx, uint32_t tag)
    {
        uint32_t chain_idx2 = AltIndex(chain_idx, tag);
        // 遍历两个可能链的每个桶
        for (int i = 0; i < chain_capacity; i++)
        {
            char *p = data_base + (chain_idx * chain_capacity + i) * bucket_size;
            if (DeleteTag(p, tag))
            {
                return true;
            }
        }
        for (int i = 0; i < chain_capacity; i++)
        {
            char *p = data_base + (chain_idx2 * chain_capacity + i) * bucket_size;
            if (DeleteTag(p, tag))
            {
                return true;
            }
        }
        return false;
    }

    /**
     * 按照条件，删除tag和对应value
     * is_src true表示需要扩展的segment，false表示新增的segment
     * 
     */
    void EraseEle(bool is_src, uint32_t actv_bit)
    {
        char *p;
        int bucket_id = 0;
        for (int i = 0; i<chain_num; i++) {
            for (int j=0; j<chain_capacity; j++) {
                p = data_base + (i * chain_capacity + j) * bucket_size;
                uint64_t delFlag = doErase(p, is_src, actv_bit);
                eraseValue(i, j, delFlag, is_src);
            }
        }
        insert_cur = 0;
    }

    void Absorb(const Segment *segment)
    {
        char *p1 = data_base;
        uint32_t len1 = (chain_capacity * bucket_size);
        char *p2 = segment->data_base;
        uint32_t len2 = (segment->chain_capacity * segment->bucket_size);

        chain_capacity += segment->chain_capacity;
        insert_cur = 0;
        ANS_MASK = ~(0xFFFFFFFF << 2 * (2 * chain_capacity * kTagsPerBucket % 16));

        total_size = chain_num * chain_capacity * bucket_size + safe_pad;
        data_base = new char[total_size];

        for (int i = 0; i < chain_num; i++)
        {
            memcpy(data_base + i * (len1 + len2), p1 + i * len1, len1);
            memcpy(data_base + i * (len1 + len2) + len1, p2 + i * len2, len2);
        }
        delete[] p1;
        delete[] temp;
        temp = new char[safe_pad_simd + (2 * chain_capacity * bucket_size + 23) / 24 * 24 + safe_pad_simd];
    }

    // const参数只能使用const方法
    /**
     * 获取segment中tag的容量
     */
    uint32_t getTagNum() const
    {
        return chain_capacity * chain_num * kTagsPerBucket;
    }

    ValueEntry **getValueSet() const
    {
        return value_set;
    }

    bool SwapValue(ValueEntry *value1, ValueEntry *value2)
    {
        ValueEntry temp(*value1);
        value1->CpFrom(*value2);
        value2->CpFrom(temp);
        return true;
    }

    /**
     * cmp 是由 1010组成的数字，其中1表示匹配成功，0表示匹配失败
     * 从0开始计数，则第1位，3位...为有效位，分别表示从1开始计数的对比的指纹1，指纹2...是否匹配
     */
    vector<int> cmp_to_tag_id(uint32_t cmp) const
    {
        vector<int> ret;
        uint32_t mask = 0x00000003;
        for (int i=0; i<16; i++) {
            if (cmp & mask) {
                ret.push_back(i);
            }
            mask = mask << 2;
        }
        return ret;
    }


    /**
     * cmp 对比的结果
     * times 16一组的比较次数
     * chain_idx 桶编号（Look传入的，未AltIndex前的，该桶及其平行溢出链排在前面）
     * tag 指纹
     */
    // void LookupValue(uint32_t cmp, int times, size_t chain_idx, uint32_t tag, vector<char*> &values) const{
    //     vector<char*> valuesP;
    //     LookupValueP(cmp, times, chain_idx, tag, valuesP);
    //     for (char *valueP : valuesP) {
    //         char *value = new char[BYTE_PER_VALUE];
    //         memcpy(value, valueP, BYTE_PER_VALUE);
    //         values.push_back(value);
    //     }
    // }

    /**
     * 加密value
     * 注意：明文需要算上 \0 的长度啊 !
     */
    void Encrypt(char *password) {
        for (int i=0; i<getTagNum(); i++) {
            ValueEntry *valueEP =  getValueP(i);
            if (valueEP->getLen() != 0) {
                valueEP->Enc(password);
            }
        }
    }

    /**
     * values传入对应tag所有的value值，（包括正真的value和碰撞的value X 现在没有碰撞了）
     * chain_idx 桶id
     * tag 指纹
     * valueE 加密后的ValueEntry
     */
    void UpdateValue(uint32_t chain_idx, uint16_t tag, ValueEntry newVE) {
        vector<char*> valuesP;
        ValueEntry *ve = LookupP(chain_idx, tag);
        ve->CpFrom(newVE);
    }

    ValueEntry getValue(int index) const {
        ValueEntry *valueP = value_set[index];
        return *valueP;
    }

    ValueEntry *getValueP(int index) const {
        ValueEntry *valueP = value_set[index];
        return valueP;
    }
    /**
     * 用于初始化的时候添加随机尾数
     */
    void AddRandom() {
        for (int i=0; i<getTagNum(); i++) {
            ValueEntry *valueEP =  getValueP(i);
            if (valueEP->getLen() != 0) {
                valueEP->SpliceRandom();
            }
        }
    }

    size_t getMemOverhead() {
        size_t overhead = 0;
        // data_base的大小
        overhead += total_size * sizeof(char);
        // 统计value_set指针的大小
        overhead += sizeof(ValueEntry*) * getTagNum();
        // 统计valueSet实际的大小
        for (int i=0; i<getTagNum(); i++) {
            overhead += value_set[i]->getMemOverhead();
        }
        // 统计其他元素的大小
        overhead += sizeof(Segment);
        return overhead;
    }
};
