#include <immintrin.h>
#include <stdint.h>
#include <stdlib.h>

#include <iostream>

#include "bamboofilter/bitsutil.h"
#include "bamboofilter/predefine.h"

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
    const uint32_t chain_num;
    uint32_t chain_capacity; // 溢出链 + 1（初始段）
    uint32_t total_size;
    uint32_t insert_cur;
    char *data_base;
    uint32_t ANS_MASK;
    char *value_set;

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

    static void doErase(char *p, bool is_src, uint32_t actv_bit)
    {
        uint64_t v = (*(uint64_t *)p) & kBucketMask;

        ((uint64_t *)p)[0] &= 0xffff000000000000;
        ((uint64_t *)p)[0] |= is_src ? ll_isl(v, actv_bit) : ll_isn(v, actv_bit);
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

    void set_value(uint32_t bucket_id, uint32_t chain_id, uint32_t tag_id, const char *value)
    {
        char *value_p = value_set + (((bucket_id * chain_capacity) + chain_id) * kTagsPerBucket + tag_id) * BYTE_PER_VALUE;
        memcpy(value_p, value, BYTE_PER_VALUE);
    }

    char *get_value(uint32_t bucket_id, uint32_t chain_id, uint32_t tag_id) const
    {
        return value_set + (((bucket_id * chain_capacity) + chain_id) * kTagsPerBucket + tag_id) * BYTE_PER_VALUE;
    }

    int cmp_to_tag_id(int cmp)
    {
        int idx = 0;
        while ((cmp = cmp >> 2))
        {
            ++idx;
        }
        return idx;
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

        uint32_t value_size = BYTE_PER_VALUE * getTagNum();
        value_set = new char[value_size];
        memset(value_set, 0, value_size);
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

        uint32_t value_size = BYTE_PER_VALUE * s.getTagNum();
        value_set = new char[value_size];
        memcpy(value_set, s.getValueSet(), value_size);
    }

    ~Segment()
    {
        delete[] data_base;
        delete[] temp;
        delete[] value_set;
    };

    /**
     * chain_idx 桶id
     * curtag 指纹
     */
    bool Insert(uint32_t chain_idx, uint32_t curtag, char *value)
    {
        char *bucket_p;
        char *value_p;
        for (uint32_t count = 0; count < MAX_CUCKOO_KICK; count++)
        {
            bucket_p = data_base + (chain_idx * chain_capacity + insert_cur) * bucket_size;

            bool kickout = count > 0;

            for (size_t tag_idx = 0; tag_idx < kTagsPerBucket; tag_idx++)
            {
                if (0 == ReadTag(bucket_p, tag_idx))
                {
                    WriteTag(bucket_p, tag_idx, curtag);
                    // 写入value
                    set_value(chain_idx, insert_cur, tag_idx, value);
                    return true;
                }
            }

            if (kickout)
            {
                size_t tag_idx = rand() % kTagsPerBucket;
                uint32_t oldtag = ReadTag(bucket_p, tag_idx);
                WriteTag(bucket_p, tag_idx, curtag);
                value_p = value_set + ((chain_idx * chain_capacity + insert_cur) * kTagsPerBucket + tag_idx) * BYTE_PER_VALUE;
                SwapValue(value, value_p);
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
            uint32_t old_valueset_len = chain_capacity * kTagsPerBucket * BYTE_PER_VALUE;
            chain_capacity++;
            uint32_t new_chain_len = chain_capacity * bucket_size;
            ANS_MASK = ~(0xFFFFFFFF << 2 * (2 * chain_capacity * kTagsPerBucket % 16));
            delete[] temp;
            temp = new char[safe_pad_simd + (2 * chain_capacity * bucket_size + 23) / 24 * 24 + safe_pad_simd];

            total_size = chain_num * chain_capacity * bucket_size + safe_pad;
            data_base = new char[total_size];
            memset(data_base, 0, total_size);

            // 重新复制value
            uint32_t new_value_len = getTagNum() * BYTE_PER_VALUE;
            char *new_value_set = new char[new_value_len];
            memset(new_value_set, 0, new_value_len);
            uint32_t new_valueset_len = chain_capacity * kTagsPerBucket * BYTE_PER_VALUE;

            for (int i = 0; i < chain_num; i++)
            {
                memcpy(data_base + i * new_chain_len, old_data_base + i * old_chain_len, old_chain_len);
                memcpy(new_value_set + i * new_valueset_len, value_set + i * old_valueset_len, old_valueset_len);
            }
            delete[] old_data_base;
            delete[] value_set;
            value_set = new_value_set;
        }
        return Insert(chain_idx, curtag, value);
    }

    /**
     * chain_idx : bucket_index
     */
    bool Lookup(uint32_t chain_idx, uint16_t tag, char *&value) const
    {
        memcpy(temp + safe_pad_simd,
               data_base + chain_idx * chain_capacity * bucket_size, // bucket_size = 6
               chain_capacity * bucket_size);
        memcpy(temp + safe_pad_simd + chain_capacity * bucket_size,
               data_base + AltIndex(chain_idx, tag) * chain_capacity * bucket_size,
               chain_capacity * bucket_size);

        char *value_set_p_0 = value_set + chain_idx * chain_capacity * kTagsPerBucket * BYTE_PER_VALUE; // 桶的起始地址
        char *value_set_p_1 = value_set + AltIndex(chain_idx, tag) * chain_capacity * kTagsPerBucket * BYTE_PER_VALUE;

        char *p = temp + safe_pad_simd;
        char *end = p + 2 * chain_capacity * bucket_size;

        int times = 0;
        __m256i _true_tag = _mm256_set1_epi16(tag); // 将tag装入16个平行的16字节中（p标量）
        int cmp = 0;
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
                value = LookupValue(cmp, times, chain_idx, tag);

                return true;
            }
            p += 24;
            ++times;
        }
        __m256i _16_tags = unpack12to16(p);

        __m256i _ans = _mm256_cmpeq_epi16(_16_tags, _true_tag);
        cmp = ANS_MASK & _mm256_movemask_epi8(_ans);
        if (cmp)
        {
            value = LookupValue(cmp, times, chain_idx, tag);
            return true;
        }
        return false;
    }

    bool Delete(uint32_t chain_idx, uint32_t tag)
    {
        uint32_t chain_idx2 = AltIndex(chain_idx, tag);
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

    void EraseEle(bool is_src, uint32_t actv_bit)
    {
        for (int i = 0; i < chain_num * chain_capacity; i++)
        {
            char *p = data_base + i * bucket_size;
            doErase(p, is_src, actv_bit);
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
    uint32_t getTagNum() const
    {
        return chain_capacity * chain_num * kTagsPerBucket;
    }

    char *getValueSet() const
    {
        return value_set;
    }

    bool SwapValue(char *value1, char *value2)
    {
        char *temp = new char[BYTE_PER_VALUE];
        memcpy(temp, value1, BYTE_PER_VALUE);
        memcpy(value1, value2, BYTE_PER_VALUE);
        memcpy(value2, temp, BYTE_PER_VALUE);
        delete[] temp;
        return true;
    }

    /**
     * cmp 对比的结果
     * times 16一组的比较次数
     * chain_idx 桶编号（Look传入的，未AltIndex前的，该桶及其平行溢出链排在前面）
     * tag 指纹
     */
    char* LookupValue(int cmp, int times, size_t chain_idx, uint32_t tag) const {
        int tag_index = (int)log2(cmp / 3) / log2(4);
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
        char *value_p = get_value(bucket_id, chain_id, tag_id);
        char *value = new char[BYTE_PER_VALUE];
        memcpy(value, value_p, BYTE_PER_VALUE);
        return value;
    }
};
