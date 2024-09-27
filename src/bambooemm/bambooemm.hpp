#ifndef BAMBOOEMM_H_
#define BAMBOOEMM_H_

#include "bamboofilter.hpp"
#include "KV.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>
using namespace std;

class BambooEMM
{

private:
    BambooFilter *bf;
    unsigned char *KI;

public:
    BambooEMM()
    {
    }
    ~BambooEMM()
    {
    }

    bool setup(int level, int n, int l, vector<KV *> mm)
    {

        bf = new BambooFilter(upperpower2(200000), 2);
        KI = LoadKey();
        for (KV *kv : mm)
        {
            unsigned char *encKey = AESEnc(KI, (unsigned char *)kv->key);
            int encKeyLen = strlen((char *)encKey);
            int tailLen = LenOfInt(kv->counter) + 2;
            char *tail = new char[tailLen];
            sprintf(tail, ",%d", kv->counter);
            unsigned char *tailUS = (unsigned char *)tail; // 指针赋值，不能提前释放tail！

            int ktLen = encKeyLen + tailLen;
            unsigned char *kt = new unsigned char[ktLen];
            memset(kt, 0, ktLen);
            memcpy(kt, encKey, encKeyLen);
            memcpy(kt + encKeyLen, tailUS, tailLen);

            bf->Insert((char *)kt);

            // //cout << "EK:";
            // //printBinary(kt, strlen((char*)kt));
            // cout << kv->key << endl;
            // cout << kt << "|";
            // unsigned char *k;
            // int c;
            // regetKT(kt, k, c);
            // cout << k << endl;
            // cout << "___________________" << endl;
        }

        return true;
    }

    void regetKT(unsigned char *kv, unsigned char *&key, int &counter)
    { // 指针的引用才能修改指针的指向！
        int kvLen = strlen((char *)kv);
        int boundary;
        for (int i = kvLen - 1; i >= 0; i--)
        { // 乱码中还是可能出现逗号的
            if (kv[i] == ',')
            {
                boundary = i;
                break;
            }
        }

        unsigned char *encKey = new unsigned char[boundary + 1];
        memset(encKey, 0, boundary + 1);
        memcpy(encKey, kv, boundary);
        // cout << "KK:";
        // printBinary(encKey, strlen((char*)encKey));

        unsigned char *counterUCStr = new unsigned char[kvLen - boundary];
        memset(counterUCStr, 0, kvLen - boundary);
        memcpy(counterUCStr, kv + boundary + 1, kvLen - boundary - 1);
        // cout << "CC:";
        // printBinary(counterUCStr, strlen((char*)counterUCStr));

        key = AESDec(KI, encKey);
        counter = atoi((char *)counterUCStr);

        // delete encKey;
        // delete counterUCStr;

        // cout << kv << " | " << key << " | " << counter << endl;
    }

    BambooFilter *getEMM()
    {
        return bf;
    }
};

#endif
