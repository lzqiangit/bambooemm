#ifndef _FILTER_POSITION_H_
#define _FILTER_POSITION_H_

#include "predefine.h"

class FilterPosition
{
public:
    uint32_t seg;
    uint32_t bucket;        
    uint32_t tag;

    FilterPosition(uint32_t seg, uint32_t bucket, uint32_t tag);
    ~FilterPosition();

    bool operator<(const FilterPosition &other) const
    {

        if (seg != other.seg)
        {
            return seg < other.seg;
        }

        int altBucketIndex = ((uint32_t)(bucket ^ tag) & ((1 << BUCKETS_PER_SEG) - 1));
        if (bucket != other.bucket &&  altBucketIndex != other.bucket)
        {
            return bucket < other.bucket;
        }
        bool ret = tag < other.tag;
        return ret;
    }

    bool operator==(const FilterPosition &other) const
    {
        int altBucketIndex = ((uint32_t)(bucket ^ tag) & ((1 << BUCKETS_PER_SEG) - 1));
 
        bool ret = seg == other.seg && ( bucket == other.bucket || altBucketIndex == other.bucket ) && tag == other.tag;
        return ret;
    }
};

FilterPosition::FilterPosition(uint32_t seg, uint32_t bucket, uint32_t tag) : seg(seg), bucket(bucket), tag(tag)
{
    
}

FilterPosition::~FilterPosition()
{
}
#endif