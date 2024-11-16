#ifndef _EMMU_HPP_
#define _EMMU_HPP_


typedef struct emmuElement
{
    int len;
    char *p;
} UElement;

class EMMu
{
private:
    int len;
    UElement *data;

    int Hash();
public:
    EMMu(/* args */);
    ~EMMu();
};

EMMu::EMMu(/* args */)
{
}

EMMu::~EMMu()
{
}

int EMMu::Hash() {
    return 1;
}
#endif