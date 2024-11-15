#ifndef _VALUE_ENTRY_H_
#define _VALUE_ENTRY_H_

typedef unsigned int uint32_t;
#include <cstring>
#include <string>
class ValueEntry
{
private:
    int len;
    char *p;
public:
    ValueEntry(/* args */);
    /**
     * len长度应该包含 \0 如果需要的话
     * 参数p注意释放
     */
    ValueEntry(int len, char *p);
    ValueEntry(const ValueEntry& other);
    ~ValueEntry();

    void SetValue(int len, char *p);
    /**
     * 复制value和p
     * 返回长度
     * 并将值复制到vp指针指向位置(函数内申请空间)
     */
    char *DuplicateValue(int &len, char *dv);

    void CpFrom(ValueEntry ve);

    /**
     * 用于初始化的时候, 直接再value的明文上面拼接值
     * appendLen 拼接value的长度
     * append 指向拼接value值的指针
     */
    void AppendValue(int appendLen, char *append);

    void erase();

    int getLen() const;
    char *getP() const;

    // 
    ValueEntry& operator=(ValueEntry &ve);

};

ValueEntry::ValueEntry()
{
    this->len = 0;
    this->p = nullptr;
}

ValueEntry::ValueEntry(int len, char*p) 
{   
    SetValue(len, p);
}

ValueEntry::ValueEntry(const ValueEntry& other) {
    /** 拷贝构造函数中不能再传递同类对象,否则会造成递归调用,死循环 */
    // 深拷贝

    len = other.getLen();
    if (len != 0) {
        p = new char[len];
        memcpy(p, other.getP(), len);
    }
    
}

ValueEntry::~ValueEntry()
{
    if (len != 0) {
        delete []p;
    }
    
}

void ValueEntry::SetValue(int len, char *p) {
    if (len != 0) {
        delete[] this->p;
    }
    this->len = len;
    this->p = new char[len];
    memset(this->p, 0, len);
    memcpy(this->p, p, len);
}

char *ValueEntry::DuplicateValue(int &len, char *dv) {
    len = this->len;
    dv = new char[len];
    memset(dv, 0, len);
    memcpy(dv, this->p, len);
}

void ValueEntry::CpFrom(ValueEntry ve) {
    if (len != 0) {
        delete[] this->p;
    }
    this->len = ve.getLen();
    this->p = new char[len];
    memset(this->p, 0, this->len);
    memcpy(this->p, ve.getP(), this->len);
}

void ValueEntry::AppendValue(int appendLen, char *append) {
    
    string val = p;
    string app = append;
    string after = val + "," + app;
    delete []p;
    len = after.length() + 1;
    p = new char[len];
    memcpy(p, (char*)after.c_str(), len);
}

void ValueEntry::erase() {
    if (len != 0) {
       len = 0;
        delete []p; 
    }
    
}

int ValueEntry::getLen() const {
    return this->len;
}
char *ValueEntry::getP() const {
    return this->p;
}

ValueEntry& ValueEntry::operator=(ValueEntry &ve){
        // 应该先判断是否有属性在堆区，如果有，需要先释放
        if(len != 0) {
            delete[] p;
            p = nullptr;
        }
        // 深拷贝
        len = ve.getLen();
        p = new char[ve.getLen()];
        memcpy(p, ve.getP(), len);
        // 实现链式编程
        return *this;
    }
#endif