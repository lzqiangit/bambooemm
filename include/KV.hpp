#ifndef KV_H_
#define KV_H_

class KV
{
public:
    char *key;
    char *value;
    int counter;
    int random;
    KV(char *key, char *value, int counter) : key(key),
                                              value(value),
                                              counter(counter)
    {
        random = 00;
    }
    ~KV()
    {
        delete key;
        delete value;
    }
};

#endif
