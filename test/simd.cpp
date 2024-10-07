#include <iostream>
#include <immintrin.h>
#include <avx512vlbwintrin.h>
#include <stdint.h>
#include <stdlib.h>
#include <cstring>
#include "utils.hpp"
#include <cmath>
using namespace std;

void store_to_memory(__m256i tag, __m256i *memory)
{
    _mm256_store_si256(memory, tag);
}

void print_m256i(__m256i v, int len)
{
    __m256i memory[1]; // 分配内存空间
    store_to_memory(v, memory);
    char *values = (char *)memory;
    for (int i = 0; i < 32/len; ++i)
    {
        printBinary(values + i*len, len);
    }
}

void set()
{
    __m256i bytegrouping = _mm256_setr_epi8(4, 5, 5, 6, 7, 8, 8, 9, 10, 11, 11, 12, 13, 14, 14, 15,
                                            0, 1, 1, 2, 3, 4, 4, 5, 6, 7, 7, 8, 9, 10, 10, 11);
    print_m256i(bytegrouping, 1);
}

void load()
{
    char *data = new char[32];
    for (int i = 0; i < 32; i++)
    {
        memset(data + i, i, 1);
        // printBinary(data+i, 1);
    }

    __m256i v = _mm256_loadu_si256((const __m256i *)(data - 4));
    print_m256i(v, 1);
}

void shuffle() {
    char *data = new char[32];
    for (int i = 0; i < 32; i++)
    {
        memset(data + i, i, 1);
        // printBinary(data+i, 1);
    }

    __m256i v = _mm256_loadu_si256((const __m256i *)(data - 4));
    print_m256i(v, 1);
    cout << "++++++++++++++++++++++++++++++++++" << endl;
    __m256i bytegrouping = _mm256_setr_epi8(4, 5, 5, 6, 7, 8, 8, 9, 10, 11, 11, 12, 13, 14, 14, 15,
                                            0, 1, 1, 2, 3, 4, 4, 5, 6, 7, 7, 8, 9, 10, 10, 11);         // 分前16个字节和后16个字节两组，按照参数中给定的 16 * 2两组数据的编号复制到返回参数中
    v = _mm256_shuffle_epi8(v, bytegrouping);
    print_m256i(v, 1);
}

__m256i unpack12to16(const char *p)
{
    __m256i v = _mm256_loadu_si256((const __m256i *)(p - 4));       // 即为从p-4开始导入32个字节数据到v中，为什么要移动四个？   256/12 = 

    const __m256i bytegrouping =
        _mm256_setr_epi8(4, 5, 5, 6, 7, 8, 8, 9, 10, 11, 11, 12, 13, 14, 14, 15,
                            0, 1, 1, 2, 3, 4, 4, 5, 6, 7, 7, 8, 9, 10, 10, 11);
    v = _mm256_shuffle_epi8(v, bytegrouping);

    __m256i hi = _mm256_srli_epi16(v, 4);
    __m256i lo = _mm256_and_si256(v, _mm256_set1_epi32(0x00000FFF));

    return _mm256_blend_epi16(lo, hi, 0b10101010);
}

void unpack() {
    char *data = new char[32];
    for (int i = 0; i < 32; i++)
    {
        memset(data + i, i, 1);
        printBinary(data+i, 1);
    }
    cout << "=======================================" << endl;
    __m256i pv = unpack12to16(data);
    print_m256i(pv, 2);
}

void srli() {
    char *data = new char[32];
    for (int i = 0; i < 32; i++)
    {
        memset(data + i, i, 1);
    }
    __m256i v = _mm256_loadu_si256((const __m256i *)(data));
    print_m256i(v, 2);
    v = _mm256_srli_epi16(v, 1);    // 以16为单位，逻辑右移，注意x86小端对齐
    cout << "---------------------------------------" << endl;
    print_m256i(v, 2);
}

void simd_and() {
    char *data = new char[32];
    for (int i = 0; i < 32; i++)
    {
        memset(data + i, 0xff, 1);
    }
    __m256i v = _mm256_loadu_si256((const __m256i *)(data));
    print_m256i(v, 4);
    cout << "***************************************" << endl;
    v = _mm256_and_si256(v, _mm256_set1_epi32(0x00000FFF));
    print_m256i(v, 4);
}
    
void blend() {
    char *data0 = new char[32];
    char *data1 = new char[32];
    for (int i = 0; i < 32; i++)
    {
        memset(data0 + i, i, 1);
        memset(data1 + i, 31-i, 1);
    }
    __m256i v0 = _mm256_loadu_si256((const __m256i *)(data0));
    __m256i v1 = _mm256_loadu_si256((const __m256i *)(data1));
    __m256i v =_mm256_blend_epi16(v0, v1, 0b01010101);
    print_m256i(v0, 2);
    cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
    print_m256i(v1, 2);
    cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
    print_m256i(v, 2);
}

void cmpeq() {
    char *data0 = new char[32];
    char *data1 = new char[32];
    for (int i = 0; i < 32; i++)
    {
        memset(data0 + i, i, 1);
        memset(data1 + i, i-1, 1);
    }
    memset(data1 + 2, 2, 1);
    memset(data1 + 3, 3, 1);
    __m256i v0 = _mm256_loadu_si256((const __m256i *)(data0));
    __m256i v1 = _mm256_loadu_si256((const __m256i *)(data1));
    __m256i _ans = _mm256_cmpeq_epi16(v0, v1);
    print_m256i(v0, 2);
    cout << "__________________________" << endl;
    print_m256i(v1, 2);
    cout << "__________________________" << endl;
    print_m256i(_ans, 2);
    cout << "__________________________" << endl;
    int r = _mm256_movemask_epi8(_ans);
    cout << r << endl;
}

int cmp_to_tag_id(int cmp) {
    return (int)log2(cmp / 3) / log2(4);
    // int idx = 0;
    // while ((cmp = cmp >> 2)) {
    //     ++idx;
    // }
    // return idx; 
}

int main(int argc, char const *argv[])
{
    uint16_t num = 123;
    uint16_t mask = 0x8000;
    for(int j=0; j<16; j++) {
        if (mask & num) {
            cout << 1;
        } else {
            cout << 0;
        }
        mask = mask >> 1;
    }
    cout << endl;
    return 0;
}
